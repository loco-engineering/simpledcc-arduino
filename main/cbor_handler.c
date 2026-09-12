#include "cbor_handler.h"
#include "esp_log.h"
#include "cbor.h"
#include "dcc_generator.h"

static const char *TAG = "CBOR_HND";

void process_cbor_message(const uint8_t *buffer, size_t length) {
    CborParser parser;
    CborValue it;
    if (cbor_parser_init(buffer, length, 0, &parser, &it) != CborNoError) {
        ESP_LOGE(TAG, "Failed to initialize CBOR parser");
        return;
    }

    if (cbor_value_is_map(&it)) {
        CborValue map_it;
        cbor_value_enter_container(&it, &map_it);

        while (!cbor_value_at_end(&map_it)) {
            if (cbor_value_is_text_string(&map_it)) {
                bool is_dcc_config = false, is_dcc = false;
                cbor_value_text_string_equals(&map_it, "dcc_config", &is_dcc_config);
                cbor_value_text_string_equals(&map_it, "dcc", &is_dcc);
                
                if (is_dcc_config) {
                    cbor_value_advance(&map_it); // advance past the key
                    if (cbor_value_is_map(&map_it)) {
                        CborValue config_it;
                        cbor_value_enter_container(&map_it, &config_it);
                        dcc_config_t config = { .pin1 = -1, .pin2 = -1, .mode = DCC_DRIVER_MODE_IN1_IN2, .enabled = false };
                        while (!cbor_value_at_end(&config_it)) {
                            if (cbor_value_is_text_string(&config_it)) {
                                bool is_pin1 = false, is_pin2 = false, is_mode = false, is_enabled = false;
                                cbor_value_text_string_equals(&config_it, "pin1", &is_pin1);
                                cbor_value_text_string_equals(&config_it, "pin2", &is_pin2);
                                cbor_value_text_string_equals(&config_it, "mode", &is_mode);
                                cbor_value_text_string_equals(&config_it, "enabled", &is_enabled);
                                
                                cbor_value_advance(&config_it);
                                if (is_pin1 && cbor_value_is_integer(&config_it)) {
                                    int pin; cbor_value_get_int(&config_it, &pin); config.pin1 = pin;
                                } else if (is_pin2 && cbor_value_is_integer(&config_it)) {
                                    int pin; cbor_value_get_int(&config_it, &pin); config.pin2 = pin;
                                } else if (is_mode && cbor_value_is_integer(&config_it)) {
                                    int mode; cbor_value_get_int(&config_it, &mode); config.mode = mode;
                                } else if (is_enabled && cbor_value_is_boolean(&config_it)) {
                                    bool enabled; cbor_value_get_boolean(&config_it, &enabled); config.enabled = enabled;
                                }
                            } else {
                                cbor_value_advance(&config_it);
                            }
                            if (!cbor_value_at_end(&config_it)) cbor_value_advance(&config_it);
                        }
                        cbor_value_leave_container(&map_it, &config_it);
                        dcc_apply_config(&config);
                    } else {
                        cbor_value_advance(&map_it);
                    }
                    continue;
                } else if (is_dcc) {
                    cbor_value_advance(&map_it); // advance past the key
                    CborType type = cbor_value_get_type(&map_it);
                    ESP_LOGI(TAG, "Parsing 'dcc' key, value type is: %d", type);
                    
                    if (type == CborByteStringType) {
                        size_t dcc_len = 0;
                        cbor_value_calculate_string_length(&map_it, &dcc_len);
                        ESP_LOGI(TAG, "CBOR byte string length: %d", (int)dcc_len);
                        if (dcc_len > 0 && dcc_len <= DCC_MAX_DATA_LEN) {
                            uint8_t dcc_data[DCC_MAX_DATA_LEN];
                            size_t actual_len = sizeof(dcc_data);
                            if (cbor_value_copy_byte_string(&map_it, dcc_data, &actual_len, NULL) == CborNoError) {
                                dcc_send_packet(dcc_data, actual_len);
                            }
                        }
                        cbor_value_advance(&map_it);
                    } else if (type == CborArrayType) {
                        size_t array_len = 0;
                        cbor_value_get_array_length(&map_it, &array_len);
                        ESP_LOGI(TAG, "CBOR array length: %d", (int)array_len);
                        if (array_len > 0 && array_len <= DCC_MAX_DATA_LEN) {
                            CborValue array_it;
                            cbor_value_enter_container(&map_it, &array_it);
                            uint8_t dcc_data[DCC_MAX_DATA_LEN];
                            size_t idx = 0;
                            while (!cbor_value_at_end(&array_it) && idx < DCC_MAX_DATA_LEN) {
                                int val = 0;
                                cbor_value_get_int(&array_it, &val);
                                dcc_data[idx++] = (uint8_t)val;
                                cbor_value_advance(&array_it);
                            }
                            dcc_send_packet(dcc_data, idx);
                            cbor_value_leave_container(&map_it, &array_it);
                        } else {
                            cbor_value_advance(&map_it);
                        }
                    } else {
                        ESP_LOGW(TAG, "Unsupported CBOR type for 'dcc': %d", type);
                        cbor_value_advance(&map_it);
                    }
                    continue;
                }
            }
            // Skip unhandled key
            cbor_value_advance(&map_it);
            // Skip unhandled value
            if (!cbor_value_at_end(&map_it)) {
                cbor_value_advance(&map_it);
            }
        }
        cbor_value_leave_container(&it, &map_it);
    }
}
