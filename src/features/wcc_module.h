#ifndef WCC_MODULE_H
#define WCC_MODULE_H

#include "esp32-hal-log.h"
#include "../config/board_config.h"
#include "./led_module.h"
#include "../../structures.h"

static const char *TAG = "HANDLER_MODULE";

#define bytes_to_u16(MSB, LSB) (((unsigned int)((unsigned char)MSB)) & 255) << 8 | (((unsigned char)LSB) & 255)

// Example bytes_to_short(bytes[1], bytes[0])
static unsigned short bytes_to_short(unsigned char byte1, unsigned char byte2)
{
    unsigned short number = (byte1 << 0) | (byte2 << 8);
    return number;
}

// Example bytes_to_long(bytes[2], bytes[1], bytes[0])
static unsigned long bytes_to_long(uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
    unsigned long number = (byte1 << 0) | (byte2 << 8) | (byte3 << 16);
    return number;
}

static unsigned long bytes_to_long_from_4_bytes(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4)
{
    unsigned long number = (byte1 << 0) | (byte2 << 8) | (byte3 << 16) | (byte4 << 24);
    return number;
}

static int compare_arrays(uint8_t a[], uint8_t b[], int n)
{
    int ii;
    for (ii = 1; ii <= n; ii++)
    {
        if (a[ii] != b[ii])
            return 0;
    }
    return 1;
}

void handle_wcc_message(uint8_t *output_buffer, size_t buffer_size)
{

    ESP_LOG_BUFFER_HEX(TAG, output_buffer, buffer_size);

    // Parse the response body, check docs for more information
    // Get WCC protocol version, 2 bytes
    unsigned short protocol_version = bytes_to_short(output_buffer[1], output_buffer[0]);
    ESP_LOGI(TAG, "WCC Protocol Version: %d", protocol_version);

    // Get Project Version, 3 bytes
    unsigned long project_version = bytes_to_long(output_buffer[4], output_buffer[3], output_buffer[2]);
    ESP_LOGI(TAG, "Project Version: %d", project_version);

    // Get Decoder Amount, 3 bytes
    unsigned long decoder_amount = bytes_to_long(output_buffer[7], output_buffer[6], output_buffer[5]);
    ESP_LOGI(TAG, "Decoders in the project: %d", decoder_amount);

    // Get Decoders from the response body

    unsigned int element_data_start_ind = 8; // Position of the first element's byte

    // Tests
    uint8_t efuse_mac_address[6];
    esp_efuse_mac_get_default(efuse_mac_address);
    ESP_LOGI(TAG, "Efuse Mac address: %02x %02x %02x %02x %02x %02x", efuse_mac_address[0], efuse_mac_address[1], efuse_mac_address[2], efuse_mac_address[3], efuse_mac_address[4], efuse_mac_address[5]);

    for (unsigned int decoder_index = 0; decoder_index < decoder_amount; ++decoder_index)
    {
        ESP_LOGI(TAG, "============================");
        ESP_LOGI(TAG, "Loading data for decoder: %d", decoder_index);

        // Get Mac Address of a decoder
        uint8_t decoder_mac_address[6];

        decoder_mac_address[0] = (uint8_t)output_buffer[element_data_start_ind++];
        decoder_mac_address[1] = (uint8_t)output_buffer[element_data_start_ind++];
        decoder_mac_address[2] = (uint8_t)output_buffer[element_data_start_ind++];
        decoder_mac_address[3] = (uint8_t)output_buffer[element_data_start_ind++];
        decoder_mac_address[4] = (uint8_t)output_buffer[element_data_start_ind++];
        decoder_mac_address[5] = (uint8_t)output_buffer[element_data_start_ind++];

        ESP_LOGI(TAG, "Mac address: %02x %02x %02x %02x %02x %02x", decoder_mac_address[0], decoder_mac_address[1], decoder_mac_address[2], decoder_mac_address[3], decoder_mac_address[4], decoder_mac_address[5]);

        // Get msg length, 4 bytes; if the mac address of this decoder where we handle this message
        //  isn't the same as this board has, we forward msg_length bytes starting from element_data_start_ind to that decoder

        unsigned long msg_length = bytes_to_long_from_4_bytes((uint8_t)output_buffer[element_data_start_ind + 3], (uint8_t)output_buffer[element_data_start_ind + 2], (uint8_t)output_buffer[element_data_start_ind + 1], (uint8_t)output_buffer[element_data_start_ind]);
        element_data_start_ind += 4;
        ESP_LOGI(TAG, "Message length: %d", msg_length);

        // ToDo: Compare mac address of this decoder where we run this code and mac address e just parsed
        // If it's not the same Mac address send a msg with msg length to that decoder without parsing
        // add code

        // NOTE: The code below should be executed only if we're parsing settings for this decoder
        // If we should forward settings to another decoder inside the newtrok we should skip parsing them

        // Clean old board settings
        if (board_settings.states != NULL)
        {
            // Iterate and clean states
            for (unsigned int state_number = 0; state_number < board_settings.state_count; ++state_number)
            {
                State state = board_settings.states[state_number];
                if (state.values != NULL)
                {
                    for (unsigned int value_number = 0; value_number < state.value_count; ++value_number)
                    {
                        Value value = state.values[value_number];
                        free(value.val);
                        value.val = NULL;
                    }

                    free(state.values);
                    state.values = NULL;
                }

                if (state.wcc_msg != NULL)
                {
                    free(state.wcc_msg);
                    state.wcc_msg = NULL;
                }
            }

            free(board_settings.states);
            board_settings.states = NULL;
        }

        // Parse States
        // Get State Count, 2 bytes
        board_settings.state_count = bytes_to_short(output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
        ESP_LOGI(TAG, "States count: %d", board_settings.state_count);
        element_data_start_ind += 2;

        // Allocate memory for States for the current element
        board_settings.states = (State *)malloc(board_settings.state_count * sizeof(State));

        // Iterate over states
        for (unsigned int state_number = 0; state_number < board_settings.state_count; ++state_number)
        {
            ESP_LOGI(TAG, "\nState number: %d", state_number);
            ESP_LOGI(TAG, "Loading values");

            // Allocate memore for State
            State state;

            state.id[0] = (uint8_t)output_buffer[element_data_start_ind++];
            state.id[1] = (uint8_t)output_buffer[element_data_start_ind++];
            state.id[2] = (uint8_t)output_buffer[element_data_start_ind++];
            state.id[3] = (uint8_t)output_buffer[element_data_start_ind++];
            state.id[4] = (uint8_t)output_buffer[element_data_start_ind++];
            state.id[5] = (uint8_t)output_buffer[element_data_start_ind++];

            state.value_count = output_buffer[element_data_start_ind];
            ESP_LOGI(TAG, "Connections to update inside this state: %d", state.value_count);
            ++element_data_start_ind;

            state.values = (Value *)malloc(state.value_count * sizeof(Value));

            for (unsigned int value_number = 0; value_number < state.value_count; ++value_number)
            {
                // Allocate memory for Value
                Value value;

                // Get connection id
                value.connection_id = output_buffer[element_data_start_ind];
                ESP_LOGI(TAG, "Connection id: %d", value.connection_id);

                ++element_data_start_ind;

                // Get value length
                value.val_length = bytes_to_short(output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
                ESP_LOGI(TAG, "Value length: %d", value.val_length);
                element_data_start_ind += 2;

                // Load value
                value.val = (uint8_t *)malloc(value.val_length);

                for (unsigned int val_index = 0; val_index < value.val_length; ++val_index)
                {
                    value.val[val_index] = output_buffer[element_data_start_ind];
                    ++element_data_start_ind;
                }

                ESP_LOGI(TAG, "Value:");
                for (int i = 0; i < value.val_length; i++)
                {
                    ESP_LOGI(TAG, "%d: %d", i, value.val[i]);
                }
                ESP_LOGI(TAG, "=======================");

                add_led_connection(0, value.connection_id, (float)(value.val[0]) / 255.0);

                state.values[value_number] = value;
            }

            // Parse if this state is active
            state.is_active = output_buffer[element_data_start_ind];
            ESP_LOGI(TAG, "Is state active %d\n", state.is_active);
            ++element_data_start_ind;

            // Get WCC message IDs for this state
            state.wcc_msg_count = output_buffer[element_data_start_ind++];
            ESP_LOGI(TAG, "WCC events for this state: %d", state.wcc_msg_count);

            state.wcc_msg = (WCC_event *)malloc(state.wcc_msg_count * sizeof(WCC_event));

            for (uint8_t msg_number = 0; msg_number < state.wcc_msg_count; ++msg_number)
            {
                // Allocate memory for WCC event
                WCC_event msg;

                for (unsigned int id_index = 0; id_index < WCC_EVENT_ID_LENGTH; ++id_index)
                {
                    msg.id[id_index] = output_buffer[element_data_start_ind++];
                }

                ESP_LOGI(TAG, "WCC msg ID:");
                for (int i = 0; i < WCC_EVENT_ID_LENGTH; i++)
                {
                    ESP_LOGI(TAG, "%d: %d", i, msg.id[i]);
                }

                msg.is_state_active = output_buffer[element_data_start_ind++];
                ESP_LOGI(TAG, "is state active: %d", msg.is_state_active);

                ESP_LOGI(TAG, "=======================");

                state.wcc_msg[msg_number] = msg;
            }

            // Get DCC packet for this state
            state.dcc_packet_count = output_buffer[element_data_start_ind++];
            ESP_LOGI(TAG, "WCC events for this state: %d", state.dcc_packet_count);

            if (state.dcc_packet_count != 0)
            {
                state.dcc_packets = (DCCPacket *)malloc(state.dcc_packet_count * sizeof(DCCPacket));

                for (uint8_t packet_number = 0; packet_number < state.dcc_packet_count; ++packet_number)
                {
                    // Allocate memory for DCC packet
                    DCCPacket dcc_packet;

                    //Set DCC packet address, 2 bytes
                    dcc_packet.address[0] = output_buffer[element_data_start_ind++];
                    dcc_packet.address[1] = output_buffer[element_data_start_ind++];

                    //Set DCC packet type
                    dcc_packet.packet_type = output_buffer[element_data_start_ind++];

                    //Set DCC packet user data
                    dcc_packet.user_data_length = output_buffer[element_data_start_ind++];

                    for (unsigned int data_index = 0; data_index < dcc_packet.user_data_length; ++data_index)
                    {
                        dcc_packet.user_data[data_index] = output_buffer[element_data_start_ind++];
                    }

                    ESP_LOGI(TAG, "DCC packet address:");
                    ESP_LOGI(TAG, "%d", bytes_to_short(dcc_packet.address[1], dcc_packet.address[0]));
                    ESP_LOGI(TAG, "DCC packet type:");
                    ESP_LOGI(TAG, "%d",dcc_packet.packet_type);
                    ESP_LOGI(TAG, "DCC packet user data:");
                    for (int i = 0; i < dcc_packet.user_data_length; i++)
                    {
                        ESP_LOGI(TAG, "%d: %d", i, dcc_packet.user_data[i]);
                    }
                    ESP_LOGI(TAG, "=======================");

                    state.dcc_packets[packet_number] = dcc_packet;
                }
            }

            board_settings.states[state_number] = state;
        }

        // Send message to a module that should handle this element
        /*if (compare_arrays(board_settings.module_mac_address, efuse_mac_address, 6) == false)
        {
            ESP_LOGI(TAG, "Sending %d bytes to a module with an address %02x %02x %02x %02x %02x %02x", element_data_start_ind - element_send_data_begin_ind, board_settings.module_mac_address[0], board_settings.module_mac_address[1], board_settings.module_mac_address[2], board_settings.module_mac_address[3], board_settings.module_mac_address[4], board_settings.module_mac_address[5]);
            mdf_err_t ret = MDF_OK;
            char *element_data = MDF_MALLOC(MWIFI_PAYLOAD_LEN); //Data that we send to modules, 1 element_data contains one element
            strncpy(element_data, output_buffer + element_send_data_begin_ind, element_data_start_ind - element_send_data_begin_ind);

            size_t size = MWIFI_PAYLOAD_LEN;
            uint8_t dest_addr[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;
            mwifi_data_type_t data_type = {0x0};
            data_type.group = false;

            ret = mwifi_write(board_settings.module_mac_address, &data_type, element_data, element_data_start_ind - element_send_data_begin_ind, true);
            ESP_LOGI(TAG, "mwifi_write result = %d", ret);

            MDF_FREE(element_data);
        }else{
            update_logic(board_settings);
        }*/
        ESP_LOGI(TAG, "============================");
    }
}

void handle_wcc_event(uint8_t *output_buffer, size_t buffer_size)
{

    Serial.println("New WCC event is received");

    ESP_LOG_BUFFER_HEX(TAG, output_buffer, buffer_size);

    WCC_event msg;
    unsigned int element_data_start_ind = 0;

    for (unsigned int id_index = 0; id_index < WCC_EVENT_ID_LENGTH; ++id_index)
    {
        msg.id[id_index] = output_buffer[element_data_start_ind++];
    }

    ESP_LOGI(TAG, "WCC msg ID:");
    for (int i = 0; i < WCC_EVENT_ID_LENGTH; i++)
    {
        ESP_LOGI(TAG, "%d: %d", i, msg.id[i]);
    }

    msg.is_state_active = output_buffer[element_data_start_ind++];
    ESP_LOGI(TAG, "is state active: %d", msg.is_state_active);

    ESP_LOGI(TAG, "============================");
}

#endif