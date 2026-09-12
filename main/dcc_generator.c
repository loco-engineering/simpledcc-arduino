#include "dcc_generator.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "DCC_GEN";

#define DCC_TIME_1_HALF_US 58
#define DCC_TIME_0_HALF_US 100
#define DCC_PREAMBLE_BITS  22

typedef enum {
    DCC_STATE_PREAMBLE,
    DCC_STATE_PACKET_START,
    DCC_STATE_DATA_BYTE,
    DCC_STATE_DATA_START,
    DCC_STATE_END_BIT
} dcc_state_t;

static dcc_config_t s_config = {
    .pin1 = -1,
    .pin2 = -1,
    .mode = DCC_DRIVER_MODE_IN1_IN2,
    .enabled = false
};

static QueueHandle_t s_packet_queue = NULL;
static gptimer_handle_t s_gptimer = NULL;
static bool s_timer_running = false;

static struct {
    dcc_state_t state;
    dcc_packet_t packet;
    uint8_t bit_idx;      // 0-7 for data byte, or 0-13 for preamble
    uint8_t byte_idx;
    uint8_t current_bit;  // The actual bit value being sent (1 or 0)
    uint8_t phase;        // 0 for first half, 1 for second half
} s_isr_ctx;

// Idle packet: 0xFF, 0x00, 0xFF
static dcc_packet_t IDLE_PACKET = {
    .data = {0xFF, 0x00, 0xFF},
    .len = 3
};

// Set output polarity based on config and phase
// Phase 0: +V to track, Phase 1: -V to track
static inline void IRAM_ATTR dcc_set_outputs(uint8_t phase) {
    if (!s_config.enabled || s_config.pin1 < 0 || s_config.pin2 < 0) {
        return;
    }
    
    if (s_config.mode == DCC_DRIVER_MODE_IN1_IN2) {
        if (phase == 0) {
            gpio_set_level(s_config.pin1, 1);
            gpio_set_level(s_config.pin2, 0);
        } else {
            gpio_set_level(s_config.pin1, 0);
            gpio_set_level(s_config.pin2, 1);
        }
    } else {
        // DIR_EN mode. pin1 = DIR, pin2 = EN
        // Keep EN high, toggle DIR
        gpio_set_level(s_config.pin2, 1); // Enable is always on when outputting
        if (phase == 0) {
            gpio_set_level(s_config.pin1, 1);
        } else {
            gpio_set_level(s_config.pin1, 0);
        }
    }
}

// Disable outputs (0V to track)
static void dcc_disable_outputs(void) {
    if (s_config.pin1 >= 0) gpio_set_level(s_config.pin1, 0);
    if (s_config.pin2 >= 0) gpio_set_level(s_config.pin2, 0);
}

// Prepare next bit to send and return its duration
static uint32_t IRAM_ATTR dcc_prepare_next_bit(void) {
    uint8_t next_bit = 1;

    switch (s_isr_ctx.state) {
        case DCC_STATE_PREAMBLE:
            next_bit = 1;
            s_isr_ctx.bit_idx++;
            if (s_isr_ctx.bit_idx >= DCC_PREAMBLE_BITS) {
                s_isr_ctx.state = DCC_STATE_PACKET_START;
            }
            break;

        case DCC_STATE_PACKET_START:
            next_bit = 0;
            s_isr_ctx.state = DCC_STATE_DATA_BYTE;
            s_isr_ctx.byte_idx = 0;
            s_isr_ctx.bit_idx = 0;
            break;

        case DCC_STATE_DATA_BYTE:
            // Send MSB first
            next_bit = (s_isr_ctx.packet.data[s_isr_ctx.byte_idx] >> (7 - s_isr_ctx.bit_idx)) & 0x01;
            s_isr_ctx.bit_idx++;
            if (s_isr_ctx.bit_idx > 7) {
                s_isr_ctx.byte_idx++;
                if (s_isr_ctx.byte_idx >= s_isr_ctx.packet.len) {
                    s_isr_ctx.state = DCC_STATE_END_BIT;
                } else {
                    s_isr_ctx.state = DCC_STATE_DATA_START;
                }
            }
            break;

        case DCC_STATE_DATA_START:
            next_bit = 0;
            s_isr_ctx.state = DCC_STATE_DATA_BYTE;
            s_isr_ctx.bit_idx = 0;
            break;

        case DCC_STATE_END_BIT:
            next_bit = 1;
            s_isr_ctx.state = DCC_STATE_PREAMBLE;
            s_isr_ctx.bit_idx = 0;
            
            // Try to load new packet from queue, else load idle packet
            if (xQueueReceiveFromISR(s_packet_queue, &s_isr_ctx.packet, NULL) != pdTRUE) {
                s_isr_ctx.packet = IDLE_PACKET;
            }
            break;
    }

    s_isr_ctx.current_bit = next_bit;
    return (next_bit == 1) ? DCC_TIME_1_HALF_US : DCC_TIME_0_HALF_US;
}

static bool IRAM_ATTR dcc_timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    // We are at the end of a half-bit
    s_isr_ctx.phase = !s_isr_ctx.phase;
    
    // Output the new phase
    dcc_set_outputs(s_isr_ctx.phase);
    
    uint32_t next_alarm_val = 0;
    
    if (s_isr_ctx.phase == 1) {
        // Second half of the current bit. Same duration as the first half.
        next_alarm_val = (s_isr_ctx.current_bit == 1) ? DCC_TIME_1_HALF_US : DCC_TIME_0_HALF_US;
    } else {
        // First half of a NEW bit. We need to transition the state machine.
        next_alarm_val = dcc_prepare_next_bit();
    }
    
    // Update alarm for next interrupt
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = edata->alarm_value + next_alarm_val,
    };
    gptimer_set_alarm_action(timer, &alarm_config);
    
    return false; // No task switch needed
}

esp_err_t dcc_init(void) {
    if (s_packet_queue == NULL) {
        s_packet_queue = xQueueCreate(10, sizeof(dcc_packet_t));
        if (s_packet_queue == NULL) {
            return ESP_ERR_NO_MEM;
        }
    }

    // Load initial idle packet
    s_isr_ctx.state = DCC_STATE_PREAMBLE;
    s_isr_ctx.bit_idx = 0;
    s_isr_ctx.phase = 0;
    s_isr_ctx.packet = IDLE_PACKET;
    s_isr_ctx.current_bit = 1; // Preamble starts with 1

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz -> 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &s_gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = dcc_timer_isr,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(s_gptimer, &cbs, NULL));

    ESP_ERROR_CHECK(gptimer_enable(s_gptimer));
    
    return ESP_OK;
}

esp_err_t dcc_apply_config(const dcc_config_t *config) {
    if (!config) return ESP_ERR_INVALID_ARG;
    
    ESP_LOGI(TAG, "Applying new DCC config: enabled=%d, mode=%d, pin1=%d, pin2=%d", 
             config->enabled, config->mode, config->pin1, config->pin2);
             
    // Stop timer while configuring
    if (s_timer_running) {
        gptimer_stop(s_gptimer);
        s_timer_running = false;
    }
    dcc_disable_outputs();
    
    // Configure pins if they are valid
    if (config->pin1 >= 0) {
        gpio_reset_pin(config->pin1);
        gpio_set_direction(config->pin1, GPIO_MODE_OUTPUT);
        gpio_set_level(config->pin1, 0);
    }
    if (config->pin2 >= 0) {
        gpio_reset_pin(config->pin2);
        gpio_set_direction(config->pin2, GPIO_MODE_OUTPUT);
        gpio_set_level(config->pin2, 0);
    }
    
    memcpy(&s_config, config, sizeof(dcc_config_t));
    
    if (s_config.enabled && s_config.pin1 >= 0 && s_config.pin2 >= 0) {
        // Reset state machine
        s_isr_ctx.state = DCC_STATE_PREAMBLE;
        s_isr_ctx.bit_idx = 0;
        s_isr_ctx.phase = 0;
        s_isr_ctx.packet = IDLE_PACKET;
        s_isr_ctx.current_bit = 1;
        
        gptimer_set_raw_count(s_gptimer, 0);
        
        gptimer_alarm_config_t alarm_config = {
            .alarm_count = DCC_TIME_1_HALF_US,
            .flags.auto_reload_on_alarm = false, // We update alarm dynamically
        };
        gptimer_set_alarm_action(s_gptimer, &alarm_config);
        
        dcc_set_outputs(0); // Phase 0
        gptimer_start(s_gptimer);
        s_timer_running = true;
    }
    
    return ESP_OK;
}

esp_err_t dcc_send_packet(const uint8_t *data, uint8_t len) {
    if (len > DCC_MAX_DATA_LEN || len == 0 || !data) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI("dcc_generator", "Queuing DCC packet, len=%d, bytes: %02X %02X %02X...", 
             len, data[0], len > 1 ? data[1] : 0, len > 2 ? data[2] : 0);
             
    dcc_packet_t pkt;
    memcpy(pkt.data, data, len);
    pkt.len = len;
    
    // NMRA standard requires packets to be sent at least 3 times.
    // We send it 5 times to ensure the locomotive decoder reliably receives it.
    for (int i = 0; i < 5; i++) {
        if (xQueueSend(s_packet_queue, &pkt, portMAX_DELAY) != pdTRUE) {
            return ESP_FAIL;
        }
    }
    
    return ESP_OK;
}
