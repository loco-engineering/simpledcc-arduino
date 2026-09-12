#include "wifi_manager.h"
#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

// --- CONFIGURATION ---
#define WIFI_AP_SSID   "SimpleDCC Station"
#define MAX_SSID_LEN   32
#define MAX_PASS_LEN   64
// ---------------------

static const char *TAG = "WIFI_MGR";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

bool wifi_ap_mode_active = false;

static bool load_wifi_credentials(char *ssid, size_t max_ssid_len, char *pass, size_t max_pass_len) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("wifi_cfg", NVS_READONLY, &my_handle);
    if (err != ESP_OK) return false;
    
    err = nvs_get_str(my_handle, "ssid", ssid, &max_ssid_len);
    if (err != ESP_OK) { nvs_close(my_handle); return false; }
    
    err = nvs_get_str(my_handle, "pass", pass, &max_pass_len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) { nvs_close(my_handle); return false; }
    
    nvs_close(my_handle);
    return true;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (!wifi_ap_mode_active) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "Retrying to connect to the AP");
        } else {
            ESP_LOGI(TAG, "Disconnected from AP, AP mode is active.");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        
        if (wifi_ap_mode_active) {
            ESP_LOGI(TAG, "Successfully connected. Turning off AP mode.");
            esp_wifi_set_mode(WIFI_MODE_STA);
            wifi_ap_mode_active = false;
        }
    }
}

void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    char ssid[MAX_SSID_LEN] = {0};
    char pass[MAX_PASS_LEN] = {0};
    bool has_creds = load_wifi_credentials(ssid, sizeof(ssid), pass, sizeof(pass));

    if (has_creds) {
        wifi_config_t wifi_config = {
            .sta = {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
        };
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
        
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        
        ESP_LOGI(TAG, "Connecting to %s...", ssid);
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(5000));
        
        if (!(bits & WIFI_CONNECTED_BIT)) {
            ESP_LOGW(TAG, "Failed to connect to %s within 5s. Starting AP mode.", ssid);
            wifi_ap_mode_active = true;
            
            wifi_config_t ap_config = {
                .ap = {
                    .ssid = WIFI_AP_SSID,
                    .ssid_len = strlen(WIFI_AP_SSID),
                    .channel = 1,
                    .password = "",
                    .max_connection = 4,
                    .authmode = WIFI_AUTH_OPEN
                },
            };
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
        } else {
            ESP_LOGI(TAG, "Successfully connected to %s", ssid);
        }
    } else {
        ESP_LOGI(TAG, "No Wi-Fi credentials found. Starting AP mode.");
        wifi_ap_mode_active = true;
        
        wifi_config_t ap_config = {
            .ap = {
                .ssid = WIFI_AP_SSID,
                .ssid_len = strlen(WIFI_AP_SSID),
                .channel = 1,
                .password = "",
                .max_connection = 4,
                .authmode = WIFI_AUTH_OPEN
            },
        };
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}
