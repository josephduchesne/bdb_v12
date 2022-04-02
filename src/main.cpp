#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <nrfx.h>
#include <nrfx_pwm.h>
#include "SEGGER_RTT.h"
#include "dshot_nrf52.h"
#include "bdb_ble.h"
#include "constants.h"

DShotPWMOutput dshot_motors;

void spamChannels(int wait, int times, int value) {
  for (int i=0;i<times; i++) {
    dshot_motors.setChannel(0,value, 0);
    dshot_motors.setChannel(1,value, 0);
    dshot_motors.setChannel(2,value, 0);
    dshot_motors.setChannel(3,value, 0);
    dshot_motors.display();
    delay(wait);
  }
}

void spamChannel(int wait, int times, int value) {
  for (int i=0;i<times; i++) {
    dshot_motors.setChannel(0,value, 0);
    dshot_motors.display();
    delay(wait);
  }
}

uint8_t get_battery_percentage() {
  uint32_t raw = analogRead(VOLTAGE_SENSE_PIN);
  float voltage = (float)raw * VOLTAGE_SENSE_VOLTS_PER_LSB;
  uint8_t charge = 101;  // didn't find on table?
  if (voltage < state_of_charge_4S[0]) {
    //SEGGER_RTT_printf(0, "Critically low battery?\n"); 
    charge = 0;
  }
  for (int i=0; i<STATE_OF_CHARGE_COUNT; i++) {
    if (voltage > state_of_charge_4S[i]) {
      charge = (uint8_t)i*STATE_OF_CHARGE_INCREMENT;
    } else {
      break; // all done
    }
  }

  int intPart = (int) voltage;
  int decimalPart = (voltage - intPart) * 100;
  //SEGGER_RTT_printf(0, "Battery Raw: %d Voltage: %d.%2d Charge: %d%%\n", raw, intPart, decimalPart, charge);
  return charge;
}

void setup() {
  delay(500);
  // Set up the 4 dshot channels 

  dshot_motors.setup(M1_PIN, M1_DIR, M2_PIN, M2_DIR, M3_PIN, M3_DIR, M4_PIN, M4_DIR);
  spamChannels(1, 10, 10);  // 3d mode on 
  
  // set up voltage sense
  analogReference(AR_INTERNAL_1_8);  // 1.8V reference with ratio 11 gives us 19.8V range
  analogReadResolution(14);  // max resolution on the NRF52 series
  pinMode(VOLTAGE_SENSE_PIN, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  ble_setup(&dshot_motors);
}
// cmds: https://bitbucket.software-quality-lab.com/projects/EIS/repos/cleanflight/browse/src/main/drivers/pwm_output.h?at=f2468fb894129eefad9ae9c53e4bbb67b15015ed
// https://github.com/bitdump/BLHeli/blob/master/BLHeli_32%20ARM/BLHeli_32%20Firmware%20specs/Digital_Cmd_Spec.txt
void loop() {
  long unsigned int start_us = micros();
  static int loop_count = 0;
  
  ble_loop();

  if ((loop_count % 50) == 0) {  // 500ms
    SEGGER_RTT_printf(0, "Loop %d ", loop_count);
    ble_update_status(get_battery_percentage());
  }

  long unsigned int loop_time_us = micros() - start_us;

  delayMicroseconds(10*1000-loop_time_us);
  loop_count++;
}
