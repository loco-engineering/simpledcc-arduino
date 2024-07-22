#ifndef GPIO_MODULE_H
#define GPIO_MODULE_H

double gpio_module_time_from_last_check = 0;
gpio_connection gpio_connections[MAX_GPIO_CONNECTIONS];
uint8_t gpio_connections_count = 0;

void set_gpio(uint8_t GPIO_NUM, uint8_t value, double duration){

    gpio_connections[gpio_connections_count].signal_type = 1; //Digital
    gpio_connections[gpio_connections_count].io_pin = GPIO_NUM;
    gpio_connections[gpio_connections_count].output_value = value;
    gpio_connections[gpio_connections_count].is_enabled = true;
    gpio_connections[gpio_connections_count].is_on = true;
    gpio_connections[gpio_connections_count].next_off = millis() + duration;

    ++gpio_connections_count;


    pinMode(GPIO_NUM, OUTPUT);
    digitalWrite(GPIO_NUM, value); // back for a motor


}

void set_pwm(uint8_t GPIO_NUM, uint8_t pwm, double duration){
    gpio_connections[gpio_connections_count].signal_type = 2; //PWM
    gpio_connections[gpio_connections_count].io_pin = GPIO_NUM;
    gpio_connections[gpio_connections_count].output_value = pwm;
    gpio_connections[gpio_connections_count].is_enabled = true;
    gpio_connections[gpio_connections_count].is_on = true;
    gpio_connections[gpio_connections_count].next_off = millis() + duration;

    ++gpio_connections_count;

    pinMode(GPIO_NUM, OUTPUT);
    analogWrite(GPIO_NUM, pwm); // back for a motor
}

void check_connections(){
    for (int i = 0; i < gpio_connections_count; ++i)
    {
        gpio_connection cur_gpio_connection = gpio_connections[i];
        if (cur_gpio_connection.is_enabled){
            if (cur_gpio_connection.next_off < gpio_module_time_from_last_check){
                //Turn off this connection
                if (cur_gpio_connection.signal_type == 1){
                    digitalWrite(cur_gpio_connection.io_pin, 0); 
                }else if (cur_gpio_connection.signal_type == 2){
                    analogWrite(cur_gpio_connection.io_pin, 0);
                }

                //Remove connection and 
                for(uint8_t rem_ind = i; rem_ind < gpio_connections_count - 1; rem_ind++){
                    gpio_connections[i] = gpio_connections[i + 1];
                }
                gpio_connections_count--;

                //After we remove a connection we should check connections again
                check_connections();
            }
        }

    }
}

void loop_gpio_module()
{
    if (millis() - gpio_module_time_from_last_check < 10){
        return;
    }

    gpio_module_time_from_last_check = millis();

    check_connections();

}

#endif

