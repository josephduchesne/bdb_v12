#pragma once

#include <dshot_nrf52.h>
#include "MotorDriver.h"
#include <Arduino.h>

class BLHeliDshotNRF52 : public MotorDriver {
  private:
    int id_;
    DShotPWMOutput* driver_;
    int reverse_delay_;
  public:
  // TODO: Move reverse delay here!
    BLHeliDshotNRF52(DShotPWMOutput* driver, int id, int reverse_delay=150) : driver_(driver), id_(id), reverse_delay_(reverse_delay) {
      //driver_->configMotor(id, ch1, ch2, pwm_ch, reverse_delay);
      //there's nothing to initsetThrottle(int channel, int8_t throttle, bool telemetry_request = false);
    }

    /**
     * @brief Turn on the motor driver and any hardware configuration needed for it
     * 
     */
    void Activate() {
      //driver_->config();
      //driver->_setThrottle(id_, 0);
      // no-op for now, maybe this should do something later?
    }

    /**
     * @brief Turn off the motor driver and any hardware configured it was using
     * 
     */
    void Deactivate() {
      driver_->setThrottle(id_, 0);
    }

    /**
     * @brief Disable the motor output
     * 
     * @param brake If true, enable braking, otherwise configure for free wheeling 
     */
    void Disable(bool brake) {
      // TODO: implement brake in the driver
      Deactivate();
    }

    /**
     * @brief Set the Speed object
     * 
     * @param speed Set speed and direction
     * @param max   The maximum speed value (for scale)
     */
    void SetSpeed(int speed, unsigned int max) {
      byte speed_u = (byte)constrain(map(speed, -max, max, -100, 100), -100, 100);
      driver_->setThrottle(id_, speed_u);
      // todo: Handling reversing here?
    }

    /**
     * @brief Update the device, and set the speed
     * 
     */
    void Update() {
      driver_->display();
    }
};