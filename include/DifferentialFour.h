#pragma once

#include "MotorDriver.h"
#include <Arduino.h>

class DifferentialFour {
    public:
        enum Position : int {
            MOTOR_FRONT_LEFT = 0,
            MOTOR_BACK_LEFT,
            MOTOR_FRONT_RIGHT,
            MOTOR_BACK_RIGHT,
            MOTOR_COUNT,
        };

        MotorDriver *motors_;

        /**
         * @brief Construct a new Four Wheel Differential object
         * 
         * @param motors An array of 4 MotorDriver objects in the order defined by DifferentialFour::Position
         */
        DifferentialFour(MotorDriver *motors, int width, int length) : motors_(motors) {
            // todo: use width, length
        }
        /**
         * @brief Turn on the motor driver and any hardware configuration needed for it
         * 
         */
        void Activate() {
            for(int i=0; i<MOTOR_COUNT; i++) {
                motors_[i].Activate(); 
            }
        }

        /**
         * @brief Turn off the motor driver and any hardware configured it was using
         * 
         */
        void Deactivate() {
            for(int i=0; i<MOTOR_COUNT; i++) {
                motors_[i].Deactivate();
            }
        }

        /**
         * @brief Disable the motor output
         * 
         * @param brake If true, enable braking, otherwise configure for free wheeling 
         */
        void Disable(bool brake) {
            for(int i=0; i<MOTOR_COUNT; i++) {
                motors_[i].Disable(brake);
            }
        }

        /**
         * @brief Set the speed using arcade parameters
         * 
         * @param speed Set speed and direction
         * @param angle Set the angular component of motion. +max indicates rotating right on the spot, while 0 indicates driving straight
         * @param max   Maximum speed/angle absolute value for scale
         */
        void SetSpeedArcade(int speed, int angle, unsigned int max, bool inverted) {
            // convert from arcade to differential
            int8_t left, right;
            int8_t speed8 = (int8_t)constrain(map(speed, 0, max, 0, 100), 0, 100);
            int8_t angle8 = (int8_t)constrain(map(angle, 0, max, 0, 100), 0, 100);
            ArcadeToDifferential(speed8, angle8, left, right);

            SetSpeed(left, right, 100, inverted);
        }

        /**
         * @brief Set the Speed object
         * 
         * @param left Set speed and direction of the left side
         * @param right Set speed and direction of the right side
         * @param max   Maximum speed/angle absolute value for scale
         */
        void SetSpeed(int left, int right, unsigned int max, bool inverted) {

            // and handle inverted + normal motor output
            if (inverted) {  // inverted, swap left and right
                int tmp = left;
                left = right;
                right = tmp;
            }

            motors_[MOTOR_FRONT_LEFT].SetSpeed(left, max);
            motors_[MOTOR_BACK_LEFT].SetSpeed(left, max);
            motors_[MOTOR_FRONT_RIGHT].SetSpeed(right, max);  
            motors_[MOTOR_BACK_RIGHT].SetSpeed(right, max);  
            
        }

        /**
         * @brief Update the device, and set the speed
         * 
         */
        void Update() {
            for(int i=0; i<MOTOR_COUNT; i++) {
                motors_[i].Update();
            }
        }

    private:
        // x and y should be -100, 100, left and right will also be
        void ArcadeToDifferential(int8_t drive, int8_t steer, int8_t &left, int8_t &right) {

            // Channel mixing math from http://home.kendra.com/mauser/joystick.html
            // Get X and Y from the Joystick, do whatever scaling and calibrating you need to do based on your hardware.
            float x = (float)steer;
            float y = (float)drive;

            float v = (100.0 - abs(x)) * (y / 100.0) + y;
            float w = (100.0 - abs(y)) * (x / 100.0) + x;

            right = (int8_t)((v + w) / 2.0);
            left = (int8_t)((v - w) / 2.0);
        }
};
