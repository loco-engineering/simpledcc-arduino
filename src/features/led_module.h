#ifndef LED_MODULE_H
#define LED_MODULE_H

PCA9955B ledd(0x3F);

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

led_connection led_connections[MAX_LED_CONNECTIONS];
uint8_t led_connections_count = 0;

void setup_led()
{
    Wire.begin(11, 10);
    ledd.begin(1.0);
}

// on_duration, off_duration, start_delay - values in milliseconds
void add_led_connection(uint8_t type, uint8_t led_pin, float pwm, unsigned long on_duration = 0, unsigned long off_duration = 0, unsigned long start_delay = 0)
{

    led_connections[led_connections_count].type = type;
    if (type == 1)
    {
        // configure LED PWM functionalitites
        ledcSetup(led_connections_count, freq, resolution);
        ledcAttachPin(led_pin, led_connections_count);
    }
    led_connections[led_connections_count].led_pin = led_pin;
    led_connections[led_connections_count].pwm = pwm;
    led_connections[led_connections_count].connection_index = led_connections_count;
    led_connections[led_connections_count].on_duration = on_duration;
    led_connections[led_connections_count].off_duration = off_duration;
    led_connections[led_connections_count].is_enabled = true;
    if (start_delay == 0)
    {
        led_connections[led_connections_count].is_on = true;
    }
    else
    {
        led_connections[led_connections_count].is_on = false;
    }
    led_connections[led_connections_count].next_on = millis() + start_delay;
    led_connections[led_connections_count].next_off = millis() + on_duration;

    ++led_connections_count;
}

double time_from_last_check = 0;

void loop_led()
{
    if (millis() - time_from_last_check < 100){
        return;
    }

    time_from_last_check = millis();

    for (int i = 0; i < led_connections_count; ++i)
    {
        led_connection cur_led_connection = led_connections[i];
        if (cur_led_connection.is_enabled)
        {

            if (led_connections[i].on_duration == 0)
            {
                if (led_connections[i].is_on)
                {
                    if (led_connections[i].type == 0)
                    {
                        ledd.pwm(led_connections[i].led_pin, led_connections[i].pwm);
                    }
                    else
                    {
                        ledcWrite(led_connections[i].connection_index, (int)(led_connections[i].pwm * 255.0));
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
                            ledd.pwm(led_connections[i].led_pin, led_connections[i].pwm);
                        }
                        else
                        {
                            ledcWrite(led_connections[i].connection_index, (int)(led_connections[i].pwm * 255.0));
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
                            ledd.pwm(led_connections[i].led_pin, 0);
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