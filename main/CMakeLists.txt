execute_process(
    COMMAND bash bundle_app.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

idf_component_register(SRCS "app_main.c" "dcc_generator.c" "wifi_manager.c" "web_server.c" "cbor_handler.c"
                       PRIV_REQUIRES nvs_flash esp_wifi esp_http_server cbor driver esp_driver_gpio esp_driver_gptimer esp_netif esp_event)
