#ifndef BDC_MOTOR_MODULE_H
#define BDC_MOTOR_MODULE_H

#include "driver/mcpwm.h"
#include "esp32-hal-log.h"

static const char *BDC_TAG = "BDC_MODULE";

uint8_t bemf_pin = 0;
uint8_t isense_pin = 0;

uint8_t cur_direction = 0; // 1 - forward, 2 - reverse, 0 - stop
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
double Kp = 2, Ki = 1.45, Kd = 1;

// We need a fast running loop, because the trains are
// very lightweight, so their speed changes very fast when
// any perturbation occurs.
int sampleTime = 80; // Lower than 80 is shorter than the loop, so the PID calculations
                     // will be wrong, don't go lower than this.

double pwm_rate = 0;
double target_rpm = 0;
double measured_rpm = 0;
double d_pwm = 0;
double requested_duty = 0;

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
  pwm_config.frequency = 15000;
  pwm_config.cmpr_a = 0;
  pwm_config.cmpr_b = 0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

  // Frequency doesn't update till this is called.
  mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, 15000);

  next_time_to_pause_mpwm = micros() + 10;
  next_time_to_start_mpwm = micros() + 2;

  // turn the PID on
  myPID.SetSampleTime(sampleTime);
  myPID.SetOutputLimits(0, 0);
  myPID.SetMode(AUTOMATIC);


  //Put a driver into a sleep mode
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);

}

void set_motor_duty_target(uint8_t duty, uint8_t direction)
{
  if (duty > target_rpm)
  {
    d_pwm = 1;
  }
  else
  {
    d_pwm = -1;
  }
  requested_duty = duty;

  cur_direction = direction;

  //ESP_LOGI(BDC_TAG, "New Motor PWM Target: %d, direction: %d", duty, direction);
}

void bdc_forward(uint8_t duty)
{

  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty);

}

void bdc_reverse(uint8_t duty)
{

  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty);

}

double next_time_to_update_target_rpm = 0;
bool is_bemf_enabled = false;

void loop_bdc_module()
{

  if (if_bdc_module_disabled == true)
  {
    return;
  }

  if (is_bemf_enabled == false){
    if (cur_direction == 1)
        {
          bdc_forward(requested_duty);
        }
        else if (cur_direction == 2)
        {
          bdc_reverse(requested_duty);
        }
        else
        {
          mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
          mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
        }
        return;
  }

  if (millis() > next_time_to_update_target_rpm)
  {
    next_time_to_update_target_rpm = millis() + 10;
    if (target_rpm != requested_duty)
    {
      target_rpm += d_pwm;
    }

    myPID.SetOutputLimits(0, target_rpm);
  }

  /*if (micros() > next_time_to_pause_mpwm && is_mpwm_is_paused == false)
  {

    next_time_to_start_mpwm = micros();
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
    {*/

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

      //Arduino PID doesn't work with target == 0 that's why we set all values to 0 manually
      if (target_rpm == 0)
      {
        target_rpm = 0;
        pwm_rate = 0;
        measured_rpm = 0;
      }
      else
      {
        //myPID.Compute(); // Most important part!
      }

      // Restart PWM
      if (is_mpwm_is_paused == true)
      {
        is_mpwm_is_paused = false;
        next_time_to_pause_mpwm = micros() + 1000 * 1000;
      }
      if (target_rpm > 2)
      {
        if (cur_direction == 1)
        {
          bdc_forward(target_rpm);
        }
        else if (cur_direction == 2)
        {
          bdc_reverse(target_rpm);
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

       //ESP_LOGI(BDC_TAG, "PWM target: %f, calculated: %f, avg rpm: %f", target_rpm, pwm_rate, measured_rpm);
    }
//  }
//}

#endif
