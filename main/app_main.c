#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "dcc_generator.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "mdns.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "Starting Simple DCC Application...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS flash corrupted or version mismatch, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Initializing Wi-Fi (STA mode)");
    wifi_init_sta();

    // Initialize mDNS
    ESP_LOGI(TAG, "Initializing mDNS");
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("simpledcc"));
    ESP_ERROR_CHECK(mdns_instance_name_set("Simple DCC Web Server"));

    // Enable motor driver sleep pin if configured
    int sleep_pin = CONFIG_DCC_SLEEP_PIN;
    if (sleep_pin >= 0) {
        ESP_LOGI(TAG, "Configuring motor driver sleep pin: %d", sleep_pin);
        gpio_reset_pin(sleep_pin);
        gpio_set_direction(sleep_pin, GPIO_MODE_OUTPUT);
        gpio_set_level(sleep_pin, 1);
    }

    // Initialize DCC Generator
    ESP_LOGI(TAG, "Initializing DCC generator");
    ESP_ERROR_CHECK(dcc_init());

    dcc_config_t dcc_cfg = {
        .pin1 = CONFIG_DCC_DEFAULT_PIN1,
        .pin2 = CONFIG_DCC_DEFAULT_PIN2,
        .mode = DCC_DRIVER_MODE_IN1_IN2,
        .enabled = true
    };
    ESP_ERROR_CHECK(dcc_apply_config(&dcc_cfg));

    ESP_LOGI(TAG, "Starting Web Server");
    if (start_webserver() == NULL) {
        ESP_LOGE(TAG, "Failed to start web server");
    } else {
        ESP_LOGI(TAG, "Web server started successfully");
    }
}
