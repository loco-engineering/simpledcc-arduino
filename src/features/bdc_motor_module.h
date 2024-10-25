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
int bemf_values[10] = {0};
int average_BEMF = 0;

#include <PID_v1.h>

// Specify the links and initial tuning parameters
//
//  Note: shall we use two sets, one agressive for honing in, and a conservative
//        for keeping at the point ?
// double Kp=2, Ki=1.5, Kd=1;
// double Kp=1.15,Ki=0.35, Kd=0.00;
// double Kp=0.25,Ki=1.31, Kd=0.42;
// double Kp=0.05, Ki=1.35, Kd=0.38;

// Very good results with those:
double Kp = 0.40, Ki = 1.45, Kd = 0;

// We need a fast running loop, because the trains are
// very lightweight, so their speed changes very fast when
// any perturbation occurs.
int sampleTime = 200; // Lower than 80 is shorter than the loop, so the PID calculations
                      // will be wrong, don't go lower than this.

// Default controller serial update rate
int updateRate = 300;

double pwm_rate = 0;
double target_rpm = 10;
double measured_rpm = 0;

/**
 *   Input : The variable we're trying to control -> Measured speed of the train
 *   Output: The variable that will be adjusted by the pid -> pwm_rate
 * Setpoint: The value we want to Input to maintain -> target_rpm
 */
PID myPID(&measured_rpm, &pwm_rate, &target_rpm, Kp, Ki, Kd, DIRECT);

void IRAM_ATTR bemf_isense_isr()
{

  if (bemf_pin != 0)
  {
    for (int i = 0; i < 10; ++i)
    {
      bemf_values[i] = analogRead(bemf_pin);
    }

    // Get the avarage BEMF
    average_BEMF = 0;
    for (int i = 0; i < 10; ++i)
    {
      average_BEMF += bemf_values[i];
    }

    average_BEMF = average_BEMF / 10;
  }
  /*
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

  if (motor_1_A_pin == 0 && motor_1_B_pin == 0)
  {
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
      // attachInterrupt(motor_1_A_pin, bemf_isense_isr, LOW);
    }
  }

  if (motor_1_B_pin != 0)
  {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, motor_1_B_pin);
    if (bemf_pin != 0 || isense_pin != 0)
    {
      // attachInterrupt(motor_1_B_pin, bemf_isense_isr, LOW);
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

  next_time_to_pause_mpwm = micros() + 10;
  next_time_to_start_mpwm = micros() + 2;

  // turn the PID on
  myPID.SetSampleTime(sampleTime);
  myPID.SetOutputLimits(0, 50);
  myPID.SetMode(AUTOMATIC);
}

void set_motor_duty_target(uint8_t duty, uint8_t direction)
{
  target_rpm = duty;
  cur_direction = direction;
}

void bdc_forward(uint8_t duty)
{

  mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state

  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state

  // mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, 12000 - duty*50);
}

void bdc_reverse(uint8_t duty)
{

  mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state

  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); // call this each time, if operator was previously in low/high state

  // mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, 12000 - duty*50);
}

void loop_bdc_module()
{

  if (if_bdc_module_disabled == true)
  {
    return;
  }

  if (micros() > next_time_to_pause_mpwm && is_mpwm_is_paused == false)
  {

    next_time_to_start_mpwm = micros() + 100;
    is_mpwm_is_paused = true;

    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);

    if (cur_direction == 1)
    {
      mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    }
    else if (cur_direction == 2)
    {
      mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    }
    else
    {
      mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
      mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    }
  }
  else
  {
    if (is_mpwm_is_paused == true && micros() > next_time_to_start_mpwm)
    {

      if (bemf_pin != 0)
      {
        for (int i = 0; i < 10; ++i)
        {
          bemf_values[i] = analogRead(bemf_pin);
        }

        // Get the avarage BEMF
        average_BEMF = 0;
        for (int i = 0; i < 10; ++i)
        {
          average_BEMF += bemf_values[i];
        }

        average_BEMF = average_BEMF / 10;
      }
      measured_rpm = average_BEMF * 100.0 / 3400.0;

      myPID.Compute(); // Most important part!

      // Restart PWM
      if (is_mpwm_is_paused == true)
      {
        is_mpwm_is_paused = false;
        next_time_to_pause_mpwm = micros() + 50 * 1000;
      }
      if (pwm_rate > 5)
      {
        if (cur_direction == 1)
        {
          bdc_forward(pwm_rate);
        }
        else if (cur_direction == 2)
        {
          bdc_reverse(pwm_rate);
        }
        else
        {
          mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
          mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
        }
      }
      else
      {
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
      }

      serial_print("BEMF: ");
      serial_print(average_BEMF);
      serial_println("");

      serial_print("NEW PWM: ");
      serial_print(pwm_rate);
      serial_print(measured_rpm);

      serial_println("");
    }
  }
}

#endif
