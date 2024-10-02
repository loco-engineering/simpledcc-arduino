#ifndef BDC_MOTOR_MODULE_H
#define BDC_MOTOR_MODULE_H

#include "driver/mcpwm.h"

uint8_t bemf_pin = 0;
uint8_t isense_pin = 0;

uint8_t cur_direction = 0; // 1 - forward, 2 - reverse, 0 - stop
uint8_t cur_duty = 0;
double next_time_to_start_mpwm = 0;
double next_time_to_pause_mpwm = 0;
bool is_mpwm_is_paused = false;
bool if_bdc_module_disabled = false;

void IRAM_ATTR bemf_isense_isr()
{

  /*if (bemf_pin != 0)
  {
    serial_println("----------------------- ");

    for (int i = 0; i < 5; ++i)
    {

      int bemf = analogRead(bemf_pin);

      serial_print("BEMF: ");
      serial_print(bemf);
      serial_println("");
    }

  }

  if (isense_pin != 0)
  {

    //serial_println("=====================");

    for (int i = 0; i < 5; ++i)
    {

      int isense = analogRead(isense_pin);

      //serial_print("iSense: ");
      //serial_print(isense);
      //serial_println("");
    }

    //serial_println("======================");
  }*/
}

void setup_bdc_module()
{

    uint8_t motor_1_A_pin = preferences_motor_1_A_pin();
  uint8_t motor_1_B_pin = preferences_motor_1_B_pin();

  if (motor_1_A_pin == 0 && motor_1_B_pin == 0){
    if_bdc_module_disabled = true;
    return;
  }

  // Setup BackEMF reading
  bemf_pin = preferences_bemf_pin();
  if (bemf_pin != 0)
  {
    analogSetPinAttenuation(bemf_pin, ADC_11db);
  }

  // Setup iSense reading
  isense_pin = preferences_isense_pin();
  if (isense_pin != 0)
  {
    analogSetPinAttenuation(isense_pin, ADC_11db);
  }

  if (motor_1_A_pin != 0)
  {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, motor_1_A_pin);
    if (bemf_pin != 0 || isense_pin != 0)
    {
      attachInterrupt(motor_1_A_pin, bemf_isense_isr, FALLING);
    }
  }

  if (motor_1_B_pin != 0)
  {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, motor_1_B_pin);
    if (bemf_pin != 0 || isense_pin != 0)
    {
      attachInterrupt(motor_1_B_pin, bemf_isense_isr, FALLING);
    }
  }

  mcpwm_config_t pwm_config = {};
  pwm_config.frequency = 12000;
  pwm_config.cmpr_a = 0;
  pwm_config.cmpr_b = 0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

  // Frequency doesn't update till this is called.
  mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, 12000);

  next_time_to_pause_mpwm = millis() + 10;
}

void bdc_forward(uint8_t duty)
{

  cur_direction = 1;
  cur_duty = duty;
  mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);

  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state

  // mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, 12000 - duty*50);
}

void bdc_reverse(uint8_t duty)
{

  cur_direction = 2;
  cur_duty = duty;

  mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);

  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state

  // mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, 12000 - duty*50);
}

void loop_bdc_module()
{

  if (if_bdc_module_disabled == true){
    return;
  }

  if (millis() > next_time_to_pause_mpwm && is_mpwm_is_paused == false)
  {
    if (is_mpwm_is_paused == false)
    {
      next_time_to_start_mpwm = millis();
      is_mpwm_is_paused = true;
      if (cur_duty > 5)
      {
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
      }
    }
  }
  else
  {
    if (millis() > next_time_to_start_mpwm)
    {
      // Restart PWM
      if (is_mpwm_is_paused == true)
      {
        is_mpwm_is_paused = false;
        next_time_to_pause_mpwm = millis() + 5;
      }
      if (cur_direction == 1)
      {
        bdc_forward(cur_duty);
      }
      else if (cur_direction == 2)
      {
        bdc_reverse(cur_duty);
      }
      else
      {
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
      }
    }
  }
}

#endif
