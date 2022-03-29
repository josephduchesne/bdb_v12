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

void setup() {
  delay(500);
  // Set up the 4 dshot channels 

  dshot_motors.setup(M1_PIN, M1_DIR, M2_PIN, M2_DIR, M3_PIN, M3_DIR, M4_PIN, M4_DIR);
  spamChannels(1, 10, 10);  // 3d mode on 
  spamChannels(1, 250, 48);
  spamChannels(1, 250, 1047);
  spamChannels(1, 250, 48);
  

  pinMode(LED_BUILTIN, OUTPUT);

  ble_setup(&dshot_motors);
}
// cmds: https://bitbucket.software-quality-lab.com/projects/EIS/repos/cleanflight/browse/src/main/drivers/pwm_output.h?at=f2468fb894129eefad9ae9c53e4bbb67b15015ed
// https://github.com/bitdump/BLHeli/blob/master/BLHeli_32%20ARM/BLHeli_32%20Firmware%20specs/Digital_Cmd_Spec.txt
void loop() {
  delay(10);
  ble_loop();
}
