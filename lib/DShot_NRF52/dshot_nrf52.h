#pragma once

#include <nrfx.h>
#include <nrfx_pwm.h>

#if (NRFX_PWM_ENABLED==0) || (NRFX_PWM0_ENABLED == 0)
#error "NRFX_PWM_ENABLED and NRFX_PWM0_ENABLED must be enabled in nrfx_config.h (found in the nrf arduino core)."
#endif

class DShotPWMOutput {
  private: 
    // 16 values required to represent the sequence
    nrf_pwm_values_individual_t seq_values[17];
    nrf_pwm_sequence_t sequence;
    nrfx_pwm_config_t config;
    bool motor_directions[4] = {false, false, false, false};
    int8_t last_motor_command[4] = {0, 0, 0, 0};
    long unsigned int last_motor_time[4] = {0, 0, 0, 0};
    const int zero_blanking_ms = 150;  // time to emit 0 between forwards and backwards motion
  public:


    DShotPWMOutput(){ }

    void setup(uint8_t m1_pin, bool m1_dir, 
               uint8_t m2_pin, bool m2_dir, 
               uint8_t m3_pin, bool m3_dir, 
               uint8_t m4_pin, bool m4_dir);  // todo: make this more configurable

    /*
        Prepare data packet, attach 0 to telemetry bit, and calculate CRC
        throttle: 11-bit data
    */
    // From: https://github.com/gueei/DShot-Arduino/blob/master/src/DShot.cpp
    uint16_t createPacket(uint16_t throttle, bool telemetry_request = false);

    void setChannel(int channel, uint16_t throttle, bool telemetry_request = false);
    // -100 to 100 throttle, accounting for motor direction setting
    void setThrottle(int channel, int8_t throttle, bool telemetry_request = false);
    

    void display();
 };
