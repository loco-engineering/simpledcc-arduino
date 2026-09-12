#include "web_server.h"
#include <string.h>
#include <ctype.h>
#include "esp_log.h"
#include "cbor_handler.h"
#include "wifi_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "web_app_data.h"

static const char *TAG = "WEB_SERVER";

static void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit((unsigned char)a) && isxdigit((unsigned char)b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

static void save_wifi_credentials(const char *ssid, const char *pass) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("wifi_cfg", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return;
    
    nvs_set_str(my_handle, "ssid", ssid);
    nvs_set_str(my_handle, "pass", pass);
    nvs_commit(my_handle);
    nvs_close(my_handle);
}

static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len > 0) {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }

        ESP_LOGI(TAG, "Received packet with length: %d", ws_pkt.len);
        if (ws_pkt.type == HTTPD_WS_TYPE_BINARY) {
            process_cbor_message(ws_pkt.payload, ws_pkt.len);
        }
    }
    free(buf);
    return ret;
}

static esp_err_t options_handler(httpd_req_t *req) {
    char origin[256];
    if (httpd_req_get_hdr_value_str(req, "Origin", origin, sizeof(origin)) == ESP_OK) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", origin);
    } else {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    }

    char req_headers[256];
    if (httpd_req_get_hdr_value_str(req, "Access-Control-Request-Headers", req_headers, sizeof(req_headers)) == ESP_OK) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", req_headers);
    } else {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Private-Network", "true");
    
    // Some browsers require 204 No Content for preflight requests, but 200 OK is generally accepted.
    // ESP-IDF httpd_resp_send with NULL sends a 200 OK with Content-Length: 0.
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// Generic handler for static files embedded in firmware
static esp_err_t static_file_handler(httpd_req_t *req) {
    const web_file_t* file = (const web_file_t*)req->user_ctx;
    httpd_resp_set_type(req, file->mime);
    
    // Add caching headers for performance (1 hour)
    httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600");
    
    // Force connection close to prevent socket starvation from keep-alives
    httpd_resp_set_hdr(req, "Connection", "close");
    
    // Send the file
    httpd_resp_send(req, (const char*)file->data, file->size);
    return ESP_OK;
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    if (wifi_ap_mode_active) {
        const char* form = 
            "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<title>Wi-Fi Setup</title><style>"
            "body{font-family:system-ui,-apple-system,sans-serif;background:linear-gradient(135deg,#f5f7fa 0%,#c3cfe2 100%);display:flex;justify-content:center;align-items:center;height:100vh;margin:0;color:#333;}"
            ".container{background:#fff;padding:2rem;border-radius:12px;box-shadow:0 4px 6px rgba(0,0,0,0.1);width:100%;max-width:400px;box-sizing:border-box;}"
            "h1{margin-top:0;font-size:1.5rem;text-align:center;color:#2c3e50;}"
            ".form-group{margin-bottom:1rem;}"
            "label{display:block;margin-bottom:.5rem;font-weight:500;}"
            "input[type=\"text\"],input[type=\"password\"]{width:100%;padding:.75rem;border:1px solid #ddd;border-radius:6px;box-sizing:border-box;font-size:1rem;transition:border-color .2s;}"
            "input[type=\"text\"]:focus,input[type=\"password\"]:focus{outline:none;border-color:#3498db;box-shadow:0 0 0 3px rgba(52,152,219,0.2);}"
            "input[type=\"submit\"]{width:100%;padding:.75rem;background:#3498db;color:#fff;border:none;border-radius:6px;font-size:1rem;font-weight:600;cursor:pointer;transition:background .2s;margin-top:1rem;}"
            "input[type=\"submit\"]:hover{background:#2980b9;}"
            "</style></head><body>"
            "<div class=\"container\">"
            "<h1>Wi-Fi Setup</h1>"
            "<form action=\"/setup_wifi\" method=\"POST\">"
            "<div class=\"form-group\"><label for=\"ssid\">Network Name (SSID)</label><input type=\"text\" id=\"ssid\" name=\"ssid\" required placeholder=\"Enter Network Name\"></div>"
            "<div class=\"form-group\"><label for=\"pass\">Password</label><input type=\"password\" id=\"pass\" name=\"pass\" placeholder=\"Enter Password\"></div>"
            "<input type=\"submit\" value=\"Connect\">"
            "</form></div></body></html>";
        httpd_resp_send(req, form, HTTPD_RESP_USE_STRLEN);
    } else {
        // Serve index.html from web_files
        for (int i = 0; i < WEB_FILES_COUNT; i++) {
            if (strcmp(web_files[i].path, "/") == 0) {
                httpd_resp_set_type(req, web_files[i].mime);
                httpd_resp_send(req, (const char*)web_files[i].data, web_files[i].size);
                return ESP_OK;
            }
        }
        httpd_resp_send_404(req);
    }
    return ESP_OK;
}

static esp_err_t setup_wifi_post_handler(httpd_req_t *req) {
    char buf[256];
    int ret, remaining = req->content_len;
    if (remaining >= sizeof(buf)) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    if ((ret = httpd_req_recv(req, buf, remaining)) <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    // Using MAX_SSID_LEN (32) and MAX_PASS_LEN (64)
    char ssid[32] = {0};
    char pass[64] = {0};
    char *ssid_start = strstr(buf, "ssid=");
    char *pass_start = strstr(buf, "pass=");
    
    if (ssid_start) {
        ssid_start += 5;
        char *ssid_end = strchr(ssid_start, '&');
        if (ssid_end) *ssid_end = '\0';
        url_decode(ssid, ssid_start);
        
        if (pass_start) {
            pass_start += 5;
            char *pass_end = strchr(pass_start, '&');
            if (pass_end) *pass_end = '\0';
            url_decode(pass, pass_start);
        }
        
        ESP_LOGI(TAG, "Saving Wi-Fi credentials. SSID: %s", ssid);
        save_wifi_credentials(ssid, pass);
        
        const char* resp = 
            "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<meta http-equiv=\"refresh\" content=\"2;url=/\">"
            "<title>Saved</title><style>"
            "body{font-family:system-ui,-apple-system,sans-serif;background:linear-gradient(135deg,#f5f7fa 0%,#c3cfe2 100%);display:flex;justify-content:center;align-items:center;height:100vh;margin:0;color:#333;}"
            ".container{background:#fff;padding:2rem;border-radius:12px;box-shadow:0 4px 6px rgba(0,0,0,0.1);text-align:center;}"
            "h1{margin:0;color:#27ae60;font-size:1.5rem;}"
            "p{margin-top:.5rem;color:#7f8c8d;}"
            "</style></head><body>"
            "<div class=\"container\">"
            "<h1>Saved Successfully!</h1>"
            "<p>Restarting device to connect...</p>"
            "</div></body></html>";
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad Request");
    }
    
    return ESP_OK;
}

httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16; // Increased to allow all static web files

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root);

        httpd_uri_t setup_wifi = {
            .uri       = "/setup_wifi",
            .method    = HTTP_POST,
            .handler   = setup_wifi_post_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &setup_wifi);

        httpd_uri_t ws = {
                .uri        = "/ws",
                .method     = HTTP_GET,
                .handler    = ws_handler,
                .user_ctx   = NULL,
                .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws);

        httpd_uri_t ws_options = {
                .uri        = "/ws",
                .method     = HTTP_OPTIONS,
                .handler    = options_handler,
                .user_ctx   = NULL
        };
        httpd_register_uri_handler(server, &ws_options);
        
        // Register static files from web_files
        for (int i = 0; i < WEB_FILES_COUNT; i++) {
            if (strcmp(web_files[i].path, "/") != 0) {
                // Since `httpd_uri_t` requires a non-const pointer in early versions or expects to copy it,
                // we should allocate or let it point. But esp-idf copies the struct internally,
                // so we can use a local struct.
                httpd_uri_t static_uri = {
                    .uri       = web_files[i].path,
                    .method    = HTTP_GET,
                    .handler   = static_file_handler,
                    .user_ctx  = (void*)&web_files[i]
                };
                httpd_register_uri_handler(server, &static_uri);
            }
        }
        
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}
