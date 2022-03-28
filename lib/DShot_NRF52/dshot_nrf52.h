#pragma once

#include <nrfx.h>
#include <nrfx_pwm.h>

#if (NRFX_PWM_ENABLED==0) || (NRFX_PWM0_ENABLED == 0)
#error "NRFX_PWM_ENABLED and NRFX_PWM0_ENABLED must be enabled in nrfx_config.h (found in the nrf arduino core)."
#endif


class DShotPWMOutput {
  private: 
    // 16 values required to represent the sequence
    nrf_pwm_values_individual_t seq_values[16];
    nrf_pwm_sequence_t sequence;
    nrfx_pwm_config_t config;
  public:


    DShotPWMOutput(){ }

    void setup();  // todo: make this more configurable

    /*
        Prepare data packet, attach 0 to telemetry bit, and calculate CRC
        throttle: 11-bit data
    */
    // From: https://github.com/gueei/DShot-Arduino/blob/master/src/DShot.cpp
    uint16_t createPacket(uint16_t throttle, bool telemetry_request = false);

    void setChannel(int channel, uint16_t throttle, bool telemetry_request = false);
    

    void display();
 };
