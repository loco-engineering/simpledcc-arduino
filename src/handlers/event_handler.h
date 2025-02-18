#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "../../structures.h"
#include "../config/board_config.h"
#include "../features/GPIO_module.h"
#include "../features/bdc_motor_module.h"
#include "../dcc_reader/dcc_module.h"

void handle_gpio_pwm(State *state, unsigned int value_number, bool isOn = true)
{

    Value value = state->values[value_number];
    uint8_t value_length = value.val_length;
    // Now we assume that val has only 1 byte
    // ToDo: parse bytes specified in val_length not just one byte

    uint8_t connection_value = value.val[0];
    uint8_t connection_id = value.connection_id;

    Connection connection = board_connections[connection_id];
    if (isOn == true)
    {
        set_pwm(connection.output_num, connection_value, value.on_duration);
    }
    else
    {
        remove_gpio(connection.output_num);
    }
}

void handle_turnout_packet(DCCPacket *received_packet, DCCPacket *dcc_packet, State *state)
{
    // Check if it's the same direction
    if (dcc_packet->user_data[0] == received_packet->user_data[0])
    {
        // Check if we should activate the state (if power == 1)
        if (received_packet->user_data[1] == 1)
        {

            if (state->values != NULL)
            {
                for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
                {
                    Value value = state->values[value_number];
                    uint8_t connection_id = value.connection_id;
                    Connection connection = board_connections[connection_id];

                    //For LED control (light signals for example, we assume that
                    //a state is "on" if direction == 1 and the state is "off" if direction == 1
                    if (connection.owner_id == LED_DRIVER_PCA9955B)
                    {
                        uint8_t value_length = value.val_length;
                        // Now we assume that val has only 1 byte
                        // ToDo: parse bytes specified in val_length not just one byte

                        uint8_t connection_value = value.val[0];

                        uint8_t id[10] = {0};
                        id[0] = dcc_packet->address[0];
                        id[1] = dcc_packet->address[1];
                        id[2] = 1; 
                        id[3] = value_number;
                        if (dcc_packet->user_data[0] == 1)
                        {
                            //play_audio_from_spiffs("level_crossing.wav", 1);
                            add_led_connection(0, connection.output_num, (float)connection_value / 255.0, value.on_duration, value.off_duration, value.start_delay, id);
                        }
                        else
                        {
                            //                            play_audio_from_spiffs("level_crossing.wav", 0);
                            remove_led_connection(0, id);
                        }
                    }
                    else
                    {
                        handle_gpio_pwm(state, value_number);
                    }
                }
            }
        }
    }
}

void handle_aspect_packet(DCCPacket *received_packet, DCCPacket *dcc_packet, State *state)
{
    // Check if it's the same direction
    bool is_on = false;
    if (dcc_packet->user_data[0] == received_packet->user_data[0])
    {
        is_on = true;
    }

    if (state->values != NULL)
    {
        for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
        {
            Value value = state->values[value_number];
            uint8_t value_length = value.val_length;
            // Now we assume that val has only 1 byte
            // ToDo: parse bytes specified in val_length not just one byte

            uint8_t connection_value = value.val[0];
            uint8_t connection_id = value.connection_id;

            Connection connection = board_connections[connection_id];

            uint8_t id[10] = {0};
            id[0] = dcc_packet->address[0];
            id[1] = dcc_packet->address[1];
            id[2] = dcc_packet->user_data[0];
            id[3] = value_number;

            if (is_on == true)
            {
                add_led_connection(0, connection.output_num, (float)connection_value / 255.0, value.on_duration, value.off_duration, value.start_delay, id);
            }
            else
            {
                remove_led_connection(0, id);
            }
        }
    }
}

void handle_speed_packet(DCCPacket *received_packet, DCCPacket *dcc_packet, State *state)
{
Serial.println("new speed packet");
    if (state->values != NULL)
    {
        for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
        {
            // Value value = state->values[value_number];
            // uint8_t value_length = value.val_length;
            // Now we assume that val has only 1 byte
            // ToDo: parse bytes specified in val_length not just one byte

            // We skip value of a state because we take speed from a DCC packet
            uint8_t speed = received_packet->user_data[0];
            uint8_t dir = received_packet->user_data[1];
            uint8_t speed_steps = received_packet->user_data[2];

            // Many command stations send speed = 1 if a train has zero speed
            if (speed == 1)
            {
                speed = 0;
            }

            uint8_t duty_cycle = speed * 100 / speed_steps;

            if (dir == DCC_DIR_REV)
            {
                set_motor_duty_target(duty_cycle, 1);
                // bdc_reverse(duty_cycle);
            }
            else
            {
                set_motor_duty_target(duty_cycle, 2);
                // bdc_forward(duty_cycle);
            }
        }
    }
}

void handle_multi_function_packet(DCCPacket *received_packet, DCCPacket *dcc_packet, State *state)
{

    uint8_t function_group = received_packet->user_data[0];
    uint8_t function_state = received_packet->user_data[1];
    uint8_t function_number = dcc_packet->user_data[0]; // the number of a function of DCC packet attached to the state

    switch (function_group)
    {
#ifdef NMRA_DCC_ENABLE_14_SPEED_STEP_MODE
    case FN_0:
        serial_print(" FN0: ");
        serial_println((function_state & FN_BIT_00) ? "1  " : "0  ");
        break;
#endif

    case FN_0_4:

        if (function_number >= 0 && function_number <= 4)
        {
            // Update GPIO depending on a function status received from a DCC packet
            /*if (Dcc.getCV(CV_29_CONFIG) & CV29_F0_LOCATION) // Only process Function 0 in this packet if we're not in Speed Step 14 Mode
            {
                serial_print(" FN 0: ");
                serial_print((function_state & FN_BIT_00) ? "1  " : "0  ");
            }*/

            uint8_t func_mask = 1 << (function_number - 1);
            if (function_number == 0)
            {
                func_mask = 0x10;
            }

            for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
            {
                handle_gpio_pwm(state, value_number, function_state & func_mask);
            }

            if (function_state & func_mask)
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" on");
                Serial.println("");
            }
            else
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" off");
                Serial.println("");
            }
        }
        break;

    case FN_5_8:
        if (function_number >= 5 && function_number <= 8)
        {

            uint8_t number_inside_group = function_number - 4;
            uint8_t func_mask = 1 << (number_inside_group - 1);

            for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
            {
                handle_gpio_pwm(state, value_number, function_state & func_mask);
            }

            if (function_state & func_mask)
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" on");
                Serial.println("");
            }
            else
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" off");
                Serial.println("");
            }
        }
        break;

    case FN_9_12:

        if (function_number >= 9 && function_number <= 12)
        {

            uint8_t number_inside_group = function_number - 8;
            uint8_t func_mask = 1 << (number_inside_group - 1);

            for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
            {
                handle_gpio_pwm(state, value_number, function_state & func_mask);
            }

            if (function_state & func_mask)
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" on");
                Serial.println("");
            }
            else
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" off");
                Serial.println("");
            }
        }
        break;

    case FN_13_20:

        if (function_number >= 13 && function_number <= 20)
        {

            uint8_t number_inside_group = function_number - 12;
            uint8_t func_mask = 1 << (number_inside_group - 1);

            for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
            {
                handle_gpio_pwm(state, value_number, function_state & func_mask);
            }

            if (function_state & func_mask)
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" on");
                Serial.println("");
            }
            else
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" off");
                Serial.println("");
            }
        }
        break;

    case FN_21_28:

        if (function_number >= 21 && function_number <= 28)
        {

            uint8_t number_inside_group = function_number - 20;
            uint8_t func_mask = 1 << (number_inside_group - 1);

            for (unsigned int value_number = 0; value_number < state->value_count; ++value_number)
            {
                handle_gpio_pwm(state, value_number, function_state & func_mask);
            }

            if (function_state & func_mask)
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" on");
                Serial.println("");
            }
            else
            {
                Serial.print("Function ");
                Serial.print(function_number);
                Serial.print(" off");
                Serial.println("");
            }
        }
        break;
    }
}

void handle_dcc_packet(DCCPacket *received_packet)
{

    if (board_settings.states != NULL)
    {

        // Iterate and clean states
        for (unsigned int state_number = 0; state_number < board_settings.state_count; ++state_number)
        {

            State state = board_settings.states[state_number];

            if (state.dcc_packets != NULL)
            {
                if (state.dcc_packet_count > 0)
                {
                    DCCPacket dcc_packet = state.dcc_packets[0];

                    // Check if DCC packet in a state has the same address and packet type as in the received DCC packet
                    if (dcc_packet.address[0] == received_packet->address[0] && dcc_packet.address[1] == received_packet->address[1] && dcc_packet.packet_type == received_packet->packet_type)
                    {

                        switch (received_packet->packet_type)
                        {
                        case 1:
                            handle_speed_packet(received_packet, &dcc_packet, &state);
                            break;
                        case 2:
                            handle_multi_function_packet(received_packet, &dcc_packet, &state);
                            break;
                        case 3:
                            handle_aspect_packet(received_packet, &dcc_packet, &state);
                            break;
                        case 4:
                            handle_turnout_packet(received_packet, &dcc_packet, &state);
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool process_dcc_raw(DCCPacket *received_packet)
{
}

void process_dcc_speed(DCCPacket *received_packet)
{
    handle_dcc_packet(received_packet);
}

void process_dcc_multi_function(DCCPacket *received_packet)
{
    handle_dcc_packet(received_packet);
}

void process_dcc_turnout(DCCPacket *received_packet)
{
    handle_dcc_packet(received_packet);
}

void process_dcc_signal(DCCPacket *received_packet)
{
    handle_dcc_packet(received_packet);
}

void process_wcc_event(WCC_event msg)
{

    if (board_settings.states != NULL)
    {

        serial_print((String) "process_wcc_event ");

        // Iterate and clean states
        for (unsigned int state_number = 0; state_number < board_settings.state_count; ++state_number)
        {

            State state = board_settings.states[state_number];

            serial_print((String) "iterate state " + state_number);

            if (state.wcc_msg_count > 0)
            {

                serial_print((String) "state.wcc_msg_count " + state.wcc_msg_count);

                WCC_event cur_wcc_ev = state.wcc_msg[0];

                bool is_event_same = true;
                for (int i = 0; i < WCC_EVENT_ID_LENGTH; i++)
                {
                    if (cur_wcc_ev.id[i] != msg.id[i])
                    {
                        is_event_same = false;
                        break;
                    }
                }

                serial_print((String) "is_event_same " + is_event_same);

                if (is_event_same == true)
                {

                    if (msg.is_state_active == true)
                    {
                        for (unsigned int value_number = 0; value_number < msg.value_count; ++value_number)
                        {
                            Value value = msg.values[value_number];
                            uint8_t value_length = value.val_length;
                            // Now we assume that val has only 1 byte
                            // ToDo: parse bytes specified in val_length not just one byte

                            uint8_t connection_value = value.val[0];
                            uint8_t connection_id = value.connection_id;
                            Connection connection = board_connections[connection_id];
                            for (uint8_t i = 0; i < connection_amount; ++i)
                            {
                                Connection local_connection = board_connections[i];
                                if (local_connection.output_num == connection.output_num)
                                {
                                    // Check the type
                                    if (local_connection.signal_types[0] == DC_MOTOR)
                                    {
                                        uint8_t direction = 1;
                                        if (connection.output_num == preferences_motor_1_B_pin())
                                        {
                                            direction = 2;
                                        }
                                        Serial.println((String) "Get WCC event for DC motor, output " + connection.output_num + " value: " + connection_value);
                                        uint8_t duty_cycle = connection_value * 100 / 255;
                                        set_motor_duty_target(duty_cycle, direction);
                                    }
                                    else
                                    {
                                        serial_print((String) "Get WCC event for PWM, output " + connection.output_num + " value: " + connection_value);
                                        add_led_connection(0, connection.output_num, (float)(connection_value) / 255.0, value.on_duration, value.off_duration, value.start_delay);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

#endif