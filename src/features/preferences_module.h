#ifndef PREFERENCES_MODULE_H
#define PREFERENCES_MODULE_H

#include "../features/spiffs_module.h"
#include "../features/wcc_module.h"

String common_preferences_file_content = "";
String board_specific_preferences_file_content = "";

void handle_wcc_message(uint8_t *output_buffer, size_t buffer_size);

const char *get_value(const char key[20], String preferences)
{

    char *target = NULL;
    char *start, *end;

    if (start = strstr(preferences.c_str(), key))
    {
        start += strlen(key);
        if (end = strstr(start, ";"))
        {
            target = (char *)malloc(end - start + 1);
            memcpy(target, start, end - start);
            target[end - start] = '\0';
        }
    }

    if (target)
        printf("%s\n", target);
    return target;
}

const char *preferences_wifi_name()
{
    return get_value("wifi_ssid=", common_preferences_file_content);
}

const char *preferences_wifi_passwd()
{
    return get_value("wifi_passwd=", common_preferences_file_content);
}

uint8_t preferences_dcc_pin()
{
    return atoi(get_value("dcc_pin=", board_specific_preferences_file_content));
}

uint8_t preferences_i2s_bck_pin()
{
    return atoi(get_value("i2s_bck_pin=", board_specific_preferences_file_content));
}

uint8_t preferences_i2s_ws_pin()
{
    return atoi(get_value("i2s_ws_pin=", board_specific_preferences_file_content));
}

uint8_t preferences_i2s_data_pin()
{
    return atoi(get_value("i2s_data_pin=", board_specific_preferences_file_content));
}

const char *preferences_board_type()
{
    return get_value("board_type=", common_preferences_file_content);
}

uint8_t preferences_motor_1_A_pin()
{
    return atoi(get_value("motor_1_A=", board_specific_preferences_file_content));
}

uint8_t preferences_motor_1_B_pin()
{
    return atoi(get_value("motor_1_B=", board_specific_preferences_file_content));
}

uint8_t preferences_bemf_pin()
{
    return atoi(get_value("bemf_pin=", board_specific_preferences_file_content));
}

uint8_t preferences_isense_pin()
{
    return atoi(get_value("isense_pin=", board_specific_preferences_file_content));
}

void init_preferences_module()
{

    common_preferences_file_content = readFile(LittleFS, "/preferences.txt");
    String board_preferences_path = String("/preferences_") + String(get_value("board_type=", common_preferences_file_content)) + String(".txt");
    board_specific_preferences_file_content = readFile(LittleFS, board_preferences_path.c_str());

    // Load WCC settings
    size_t wcc_data_len = 0;
    uint8_t *wcc_settings_data = read_wcc_settings(LittleFS, &wcc_data_len);
    if (wcc_settings_data != NULL)
    {
        handle_wcc_message(wcc_settings_data, wcc_data_len);
    }
    free(wcc_settings_data);
}

#endif