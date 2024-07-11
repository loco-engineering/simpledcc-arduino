#ifndef WCC_MODULE_H
#define WCC_MODULE_H

#include "esp32-hal-log.h"
#include "../config/board_config.h"
#include "./led_module.h"

typedef struct
{
    uint8_t *val;
    unsigned short val_length;
    uint8_t connection_id;
} Value;

typedef struct
{
    uint8_t id[6];
    Value *values;
    uint8_t value_count;
    bool is_active;
} State;

typedef struct
{
    Connection *connections;
    State *states;
    uint8_t module_mac_address[6];
    uint8_t connection_count;
    uint8_t state_count;
} loco_element_settings;

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
        //  isn't the same as we just loaded, we will send the cut message to that decoder

        unsigned long msg_length = bytes_to_long_from_4_bytes((uint8_t)output_buffer[element_data_start_ind + 3], (uint8_t)output_buffer[element_data_start_ind + 2], (uint8_t)output_buffer[element_data_start_ind + 1], (uint8_t)output_buffer[element_data_start_ind]);
        element_data_start_ind += 4;
        ESP_LOGI(TAG, "Message length: %d", msg_length);

        // ToDo: Compare mac address of this decoder where we run this code and mac address e just parsed
        // If it's not the same Mac address send a msg with msg length to that decoder without parsing
        // add code
        loco_element_settings current_element;

        // Parse States
        // Get State Count, 2 bytes
        current_element.state_count = bytes_to_short(output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
        ESP_LOGI(TAG, "States count: %d", current_element.state_count);
        element_data_start_ind += 2;

        // Allocate memory for States for the current element
        current_element.states = (State *)malloc(current_element.state_count * sizeof(State));

        // Iterate over states
        for (unsigned int state_number = 0; state_number < current_element.state_count; ++state_number)
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

                add_led_connection(0, value.connection_id, (float) (value.val[0])/255.0);


                state.values[value_number] = value;

            }

            // Parse if this state is active
            state.is_active = output_buffer[element_data_start_ind];
            ESP_LOGI(TAG, "Is state active %d\n", state.is_active);
            ++element_data_start_ind;

            current_element.states[state_number] = state;
        }

        // Iterate over handlers
        unsigned short handler_count = bytes_to_short(output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
        ESP_LOGI(TAG, "Handlers count: %d", handler_count);
        element_data_start_ind += 2;

        for (unsigned int handler_number = 0; handler_number < handler_count; ++handler_number)
        {
            ESP_LOGI(TAG, "\nHandler number: %d", handler_number);
            ESP_LOGI(TAG, "Loading values");

            // Get Event Id that this handler can handle
            unsigned long event_id = bytes_to_long(output_buffer[element_data_start_ind + 2], output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
            ESP_LOGI(TAG, "Event Id: %d", event_id);
            element_data_start_ind += 3;

            // Get State Id that should be updated after this event is received
            // The state id is the sequence number of a state in the array with states
            unsigned short state_id = bytes_to_short(output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
            ESP_LOGI(TAG, "State Id: %d", state_id);
            element_data_start_ind += 2;

            // Get info if the state should be enabled or disabled
            unsigned short state_status = output_buffer[element_data_start_ind];
            ESP_LOGI(TAG, "State Status: %d", state_status);
            ++element_data_start_ind;
        }

        // Iterate over events that this module can send
        unsigned short event_count = bytes_to_short(output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
        ESP_LOGI(TAG, "Event amount: %d", event_count);
        element_data_start_ind += 2;

        for (unsigned int event_number = 0; event_number < event_count; ++event_number)
        {
            ESP_LOGI(TAG, "\nEvent number: %d", event_number);
            ESP_LOGI(TAG, "Loading events");

            // Get Event Id that this handler can handle
            unsigned long event_id = bytes_to_long(output_buffer[element_data_start_ind + 2], output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
            ESP_LOGI(TAG, "Event Id: %d", event_id);
            element_data_start_ind += 3;

            // Get Event Type
            unsigned short event_type = output_buffer[element_data_start_ind];
            ESP_LOGI(TAG, "Event Type: %d", event_type);
            ++element_data_start_ind;

            // Load condition when the module should send this event
            // Load value length
            unsigned short condition_value_length = bytes_to_short(output_buffer[element_data_start_ind + 1], output_buffer[element_data_start_ind]);
            ESP_LOGI(TAG, "Condition value length: %d", condition_value_length);
            element_data_start_ind += 2;

            // Load condition value
            Value val_object;
            val_object.val_length = condition_value_length;
            val_object.val = (uint8_t *)malloc(val_object.val_length);

            for (unsigned int val_index = 0; val_index < condition_value_length; ++val_index)
            {
                val_object.val[val_index] = output_buffer[element_data_start_ind];
                ++element_data_start_ind;
            }

            ESP_LOGI(TAG, "Condition value:");
            ESP_LOG_BUFFER_HEX(TAG, val_object.val, val_object.val_length);
            ESP_LOGI(TAG, "=======================");
        }

        // Send message to a module that should handle this element
        /*if (compare_arrays(current_element.module_mac_address, efuse_mac_address, 6) == false)
        {
            ESP_LOGI(TAG, "Sending %d bytes to a module with an address %02x %02x %02x %02x %02x %02x", element_data_start_ind - element_send_data_begin_ind, current_element.module_mac_address[0], current_element.module_mac_address[1], current_element.module_mac_address[2], current_element.module_mac_address[3], current_element.module_mac_address[4], current_element.module_mac_address[5]);
            mdf_err_t ret = MDF_OK;
            char *element_data = MDF_MALLOC(MWIFI_PAYLOAD_LEN); //Data that we send to modules, 1 element_data contains one element
            strncpy(element_data, output_buffer + element_send_data_begin_ind, element_data_start_ind - element_send_data_begin_ind);

            size_t size = MWIFI_PAYLOAD_LEN;
            uint8_t dest_addr[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;
            mwifi_data_type_t data_type = {0x0};
            data_type.group = false;

            ret = mwifi_write(current_element.module_mac_address, &data_type, element_data, element_data_start_ind - element_send_data_begin_ind, true);
            ESP_LOGI(TAG, "mwifi_write result = %d", ret);

            MDF_FREE(element_data);
        }else{
            update_logic(current_element);
        }*/
        ESP_LOGI(TAG, "============================");
    }
}

#endif