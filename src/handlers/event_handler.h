#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "../../structures.h"
#include "../config/board_config.h"
#include "../features/GPIO_module.h"

bool process_dcc_raw(DCCPacket *received_packet)
{
}

void process_dcc_turnout(DCCPacket *received_packet)
{

    Serial.println("1");

    if (board_settings.states != NULL)
    {
        Serial.println("2");

        // Iterate and clean states
        for (unsigned int state_number = 0; state_number < board_settings.state_count; ++state_number)
        {
            Serial.println("4");

            State state = board_settings.states[state_number];

            if (state.dcc_packets != NULL)
            {
                if (state.dcc_packet_count > 0)
                {
                    DCCPacket dcc_packet = state.dcc_packets[0];
                    Serial.println("5");

                    // Check if DCC packet in a state has the same address and packet type as in the received DCC packet
                    if (dcc_packet.address[0] == received_packet->address[0] && dcc_packet.address[1] == received_packet->address[1] && dcc_packet.packet_type == received_packet->packet_type)
                    {
                        Serial.println("6");

                        // Check if it's the same direction
                        if (dcc_packet.user_data[0] == received_packet->user_data[0])
                        {
                            // Check if we should activate the state (if power == 1)
                            if (received_packet->user_data[1] == 1)
                            {
                                Serial.println("Enable");

                                if (state.values != NULL)
                                {
                                    for (unsigned int value_number = 0; value_number < state.value_count; ++value_number)
                                    {
                                        Value value = state.values[value_number];
                                        uint8_t value_length = value.val_length;
                                        // Now we assume that val has only 1 byte
                                        // ToDo: parse bytes specified in val_length not just one byte

                                        uint8_t connection_value = value.val[0];
                                        uint8_t connection_id = value.connection_id;
                                        Connection connection = board_connections[connection_id];
                                        set_pwm(connection.output_num, connection_value, value.on_duration);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Serial.println("3");
}

void process_wcc_event(WCC_event msg)
{

    Serial.println("1");

    if (board_settings.states != NULL)
    {
        Serial.println("2");

        // Iterate and clean states
        for (unsigned int state_number = 0; state_number < board_settings.state_count; ++state_number)
        {
            Serial.println("4");

            State state = board_settings.states[state_number];

            if (state.wcc_msg_count > 0)
            {

                WCC_event cur_wcc_ev = state.wcc_msg[0];
                Serial.println("5");

                bool is_event_same = true;
                for (int i = 0; i < WCC_EVENT_ID_LENGTH; i++)
                {
                    if (cur_wcc_ev.id[i] != msg.id[i])
                    {
                        is_event_same = false;
                        break;
                    }
                }

                if (is_event_same == true)
                {
                    if (msg.is_state_active == true)
                    {
                        for (unsigned int value_number = 0; value_number < state.value_count; ++value_number)
                        {
                            Value value = state.values[value_number];
                            uint8_t value_length = value.val_length;
                            // Now we assume that val has only 1 byte
                            // ToDo: parse bytes specified in val_length not just one byte

                            uint8_t connection_value = value.val[0];
                            uint8_t connection_id = value.connection_id;
                            Connection connection = board_connections[connection_id];
                            add_led_connection(0, connection.output_num, (float)(connection_value) / 255.0);

                        }
                    }
                }
            }
        }
    }
}

#endif