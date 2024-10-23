#ifndef LED_MODULE_H
#define LED_MODULE_H

PCA9955B ledd(0x3F);

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

gpio_connection led_connections[MAX_LED_CONNECTIONS];
uint8_t led_connections_count = 0;

int compareArrays(uint8_t a[], uint8_t b[], int n)
{
    for (uint8_t ii = 0; ii < n; ii++)
    {
        if (a[ii] != b[ii])
            return 0;
    }
    return 1;
}

void setup_led()
{
    Wire.begin(11, 10);
    ledd.begin(0.1);
}

// on_duration, off_duration, start_delay - values in milliseconds
void add_led_connection(uint8_t type, uint8_t led_pin, float pwm, unsigned long on_duration = 0, unsigned long off_duration = 0, unsigned long start_delay = 0, uint8_t *id = 0)
{
    if (id == 0)
    {
        Serial.print("Id of LED connection cannot be 0\n");
        return;
    }

    // Check if we already added this connection
    bool is_LED_added_already = false;
    uint8_t connection_index = led_connections_count;
    for (int i = 0; i < led_connections_count; ++i)
    {
        gpio_connection cur_led_connection = led_connections[i];

        if (compareArrays(cur_led_connection.id, id, 10) == true)
        {
            is_LED_added_already = true;
            connection_index = i;
        }
    }

    if (is_LED_added_already == false)
    {
        led_connections[connection_index].type = type;
        if (type == 1)
        {
            // configure LED PWM functionalitites
            ledcSetup(connection_index, freq, resolution);
            ledcAttachPin(led_pin, connection_index);
        }

        led_connections[connection_index].io_pin = led_pin;
        led_connections[connection_index].connection_index = connection_index;

        for (uint8_t id_ind = 0; id_ind < 10; ++id_ind)
        {
            led_connections[connection_index].id[id_ind] = id[id_ind];
        }
    }

    led_connections[connection_index].output_value = pwm;

    led_connections[connection_index].on_duration = on_duration;
    led_connections[connection_index].off_duration = off_duration;
    led_connections[connection_index].is_enabled = true;
    if (start_delay == 0)
    {
        led_connections[connection_index].is_on = true;
    }
    else
    {
        led_connections[connection_index].is_on = false;
    }

    led_connections[connection_index].next_on = millis() + start_delay;
    led_connections[connection_index].next_off = millis() + on_duration;

    if (is_LED_added_already == false)
    {
        ++led_connections_count;
    }
}

void remove_led_connection(uint8_t type, uint8_t *id = 0)
{
    if (id == 0)
    {
        Serial.print("remove_led_connection: Id of LED connection cannot be 0\n");
        return;
    }

    for (int i = 0; i < led_connections_count; ++i)
    {
        gpio_connection cur_led_connection = led_connections[i];

        if (compareArrays(cur_led_connection.id, id, 10) == true)
        {
            // Turn off this connection
            if (led_connections[i].type == 0)
            {
                ledd.pwm(led_connections[i].io_pin, 0);
            }
            else if (led_connections[i].type == 1)
            {
                ledcWrite(led_connections[i].connection_index, 0);
            }

            // Reset LED
            led_connections[i].io_pin = 0;
            led_connections[i].output_value = 0;
            led_connections[i].connection_index = 0;
            for (uint8_t id_ind = 0; id_ind < 10; ++id_ind)
            {
                led_connections[i].id[id_ind] = 0;
            }

            // Remove connection and
            for (uint8_t rem_ind = i; rem_ind < led_connections_count - 1; rem_ind++)
            {
                led_connections[rem_ind] = led_connections[rem_ind + 1];
            }
            led_connections_count--;
            return;
        }
    }
}

double time_from_last_check = 0;

void loop_led()
{
    if (millis() - time_from_last_check < 200)
    {
        return;
    }

    time_from_last_check = millis();

    for (int i = 0; i < led_connections_count; ++i)
    {
        gpio_connection cur_led_connection = led_connections[i];
        if (cur_led_connection.is_enabled)
        {

            if (led_connections[i].on_duration == 0)
            {
                if (led_connections[i].is_on)
                {
                    if (led_connections[i].type == 0)
                    {
                        ledd.pwm(led_connections[i].io_pin, led_connections[i].output_value);
                    }
                    else
                    {
                        ledcWrite(led_connections[i].connection_index, (int)(led_connections[i].output_value * 255.0));
                    }
                }
            }
            else
            {
                if (led_connections[i].is_on)
                {
                    if (millis() < led_connections[i].next_off)
                    {
                        if (led_connections[i].type == 0)
                        {
                            ledd.pwm(led_connections[i].io_pin, led_connections[i].output_value);
                        }
                        else
                        {
                            ledcWrite(led_connections[i].connection_index, (int)(led_connections[i].output_value * 255.0));
                        }
                    }
                    else
                    {
                        Serial.println("off");

                        led_connections[i].is_on = false;
                        led_connections[i].next_on = millis() + led_connections[i].off_duration;
                    }
                }
                else
                {
                    if (millis() < led_connections[i].next_on)
                    {
                        if (led_connections[i].type == 0)
                        {
                            ledd.pwm(led_connections[i].io_pin, 0);
                        }
                        else
                        {
                            ledcWrite(led_connections[i].connection_index, 0);
                        }
                    }
                    else
                    {
                        Serial.println("on");
                        led_connections[i].is_on = true;
                        led_connections[i].next_off = millis() + led_connections[i].on_duration;
                    }
                }
            }
        }
    }
}

#endif