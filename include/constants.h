#pragma once

// Drive Motors

#define M1_PIN 30
#define M2_PIN 29
#define M3_PIN  9
#define M4_PIN 10

#define CW 0
#define CCW 1
#define M1_DIR CW
#define M2_DIR CW
#define M3_DIR CW
#define M4_DIR CW



// misc
#define VOLTAGE_SENSE_PIN 28
// ratio: 11.0 voltage divider, 1.8V range, 2^14=16384 bit range
#define VOLTAGE_SENSE_VOLTS_PER_LSB ((11.0f*1.8f)/16384.0f)
#define STATE_OF_CHARGE_INCREMENT 5
#define STATE_OF_CHARGE_COUNT 21
const float state_of_charge_critical = 15;
// https://blog.ampow.com/lipo-voltage-chart/
const float state_of_charge_4S[21] = {
    13.09, 14.43, 14.75, 14.83, 14.91, 14.99, 15.06, 15.14, 15.18, 15.26, 15.34, 15.42, 15.5, 15.66, 15.81, 15.93, 16.09, 16.33, 16.45, 16.6, 16.8
    };
