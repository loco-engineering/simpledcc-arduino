#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DCC_MAX_DATA_LEN 6

typedef enum {
    DCC_DRIVER_MODE_IN1_IN2 = 0,
    DCC_DRIVER_MODE_DIR_EN  = 1
} dcc_driver_mode_t;

typedef struct {
    int pin1; // IN1 or DIR
    int pin2; // IN2 or EN
    dcc_driver_mode_t mode;
    bool enabled;
} dcc_config_t;

typedef struct {
    uint8_t data[DCC_MAX_DATA_LEN];
    uint8_t len;
} dcc_packet_t;

/**
 * @brief Initialize the DCC generator (queues, timer)
 */
esp_err_t dcc_init(void);

/**
 * @brief Apply a new configuration to the DCC generator.
 *        If enabled is true, starts the timer and outputs DCC signal.
 *        If false, stops the timer and sets outputs to low.
 */
esp_err_t dcc_apply_config(const dcc_config_t *config);

/**
 * @brief Enqueue a new raw DCC packet to be sent to the track.
 */
esp_err_t dcc_send_packet(const uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif
