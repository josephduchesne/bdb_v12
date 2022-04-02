#pragma once

class Drivetrain {
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
         * @brief Set the speed using arcade parameters
         * 
         * @param speed Set speed and direction
         * @param angle Set the angular component of motion. +max indicates rotating right on the spot, while 0 indicates driving straight
         * @param max   Maximum speed/angle absolute value for scale
         */
        void SetSpeedArcade(int speed, int angle, unsigned int max, bool inverted) = 0;

        /**
         * @brief Set the Speed object
         * 
         * @param left Set speed and direction of the left side
         * @param right Set speed and direction of the right side
         * @param max   Maximum speed/angle absolute value for scale
         */
        void SetSpeed(int left, int right, unsigned int max, bool inverted) = 0;

        /**
         * @brief Update the device, and set the speed
         * 
         */
        virtual void Update() = 0;
};
