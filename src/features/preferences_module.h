#ifndef PREFERENCES_MODULE_H
#define PREFERENCES_MODULE_H

#include "../features/spiffs_module.h"
#include "../features/wcc_module.h"

String common_preferences_file_content = "";
String board_specific_preferences_file_content = "";
const int MAX_PREF_VALUE_LENGTH = 30;

void handle_wcc_message(uint8_t *output_buffer, size_t buffer_size);

const char *get_value(const char key[20], String preferences, char *value)
{

    //char target = NULL;
    char *start, *end;

    if (start = strstr(preferences.c_str(), key))
    {
        start += strlen(key);
        if (end = strstr(start, ";"))
        {
            //target = (char *)malloc(end - start + 1);
            size_t length = min(end - start, MAX_PREF_VALUE_LENGTH - 1);
            for (uint8_t i = 0; i < length; ++i){
                value[i] = start[i];
            }

            value[length] = '\0';
        }
    }

    if (value)
        printf("%s\n", value);

    return value;
}

const char *preferences_wifi_name(char *value)
{
    return get_value("wifi_ssid=", common_preferences_file_content, value);
}

const char *preferences_wifi_passwd(char *value)
{
    return get_value("wifi_passwd=", common_preferences_file_content, value);
}

uint8_t preferences_dcc_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("dcc_pin=", board_specific_preferences_file_content, value));
}

uint8_t preferences_i2s_bck_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("i2s_bck_pin=", board_specific_preferences_file_content, value));
}

uint8_t preferences_i2s_ws_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("i2s_ws_pin=", board_specific_preferences_file_content, value));
}

uint8_t preferences_i2s_data_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("i2s_data_pin=", board_specific_preferences_file_content, value));
}

const char *preferences_board_type(char *value)
{
    return get_value("board_type=", common_preferences_file_content, value);
}

uint8_t preferences_motor_1_A_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("motor_1_A=", board_specific_preferences_file_content, value));
}

uint8_t preferences_motor_1_B_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("motor_1_B=", board_specific_preferences_file_content, value));
}

uint8_t preferences_bemf_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("bemf_pin=", board_specific_preferences_file_content, value));
}

uint8_t preferences_isense_pin()
{
    char value[MAX_PREF_VALUE_LENGTH];
    return atoi(get_value("isense_pin=", board_specific_preferences_file_content, value));
}

void init_preferences_module()
{
    char value[MAX_PREF_VALUE_LENGTH];
    common_preferences_file_content = readFile(LittleFS, "/preferences.txt");
    String board_preferences_path = String("/preferences_") + String(get_value("board_type=", common_preferences_file_content, value)) + String(".txt");
            Serial.print(board_preferences_path);

    
    board_specific_preferences_file_content = readFile(LittleFS, board_preferences_path.c_str());

    // Load WCC settings
    size_t wcc_data_len = 0;
    uint8_t wcc_settings_data[1000];
    read_wcc_settings(LittleFS, wcc_settings_data, &wcc_data_len);
    if (wcc_settings_data != NULL)
    {
        Serial.println("((()))");
        handle_wcc_message(wcc_settings_data, wcc_data_len);
    }
}

#endif