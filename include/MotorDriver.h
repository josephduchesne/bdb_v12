#pragma once

class MotorDriver {
    public:
        /**
         * @brief Turn on the motor driver and any hardware configuration needed for it
         * 
         */
        virtual void Activate() = 0;

        /**
         * @brief Turn off the motor driver and any hardware configured it was using
         * 
         */
        virtual void Deactivate() = 0;

        /**
         * @brief Disable the motor output
         * 
         * @param brake If true, enable braking, otherwise configure for free wheeling 
         */
        virtual void Disable(bool brake) = 0;

        /**
         * @brief Set the Speed object
         * 
         * @param speed Set speed and direction, negative number for reverse
         * @param max   The maximum speed value (for scale)
         */
        virtual void SetSpeed(int speed, unsigned int max) = 0;

        /**
         * @brief Update the device, and set the speed
         * 
         */
        virtual void Update() = 0;
};
