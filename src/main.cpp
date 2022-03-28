#include <Arduino.h>
#include <Servo.h>

Servo ESC;     // create servo object to control the ESC
#include <stdio.h>
#include <string.h>
//#include <nrfx.h>
//#include <nrfx_pwm.h>
#include "SEGGER_RTT.h"
//#include "dshot_nrf52.h"
//#include "bdb_ble.h"

// from: KISS_telemetry_protocol.pdf
typedef struct {
   int8_t temperature;    // [0] = Temperature (C)
   uint16_t voltage;      // [1:2] = Voltage*100
   uint16_t current;      // [3:4] = Amps*100
   uint16_t consumption;  // [5:6] = mAh
   uint16_t erpm;         // [7-8] = ERpm/100 = RPM*Poles/100
   uint8_t crc;           // [9] = CRC
} kiss_telemetry;



//DShotPWMOutput dshot_motors;


// From KISS_telemetry_protocol.pdf
uint8_t update_crc8(uint8_t crc, uint8_t crc_seed){
  uint8_t crc_u, i;
  crc_u = crc;
  crc_u ^= crc_seed;
  for ( i=0; i<8; i++) crc_u = ( crc_u & 0x80 ) ? 0x7 ^ ( crc_u << 1 ) : ( crc_u << 1 );
  return (crc_u);
}
uint8_t get_crc8(uint8_t *Buf, uint8_t BufLen){
  uint8_t crc = 0, i;
  for( i=0; i<BufLen; i++) crc = update_crc8(Buf[i], crc);
  return (crc);
}

void print_telemetry(uint8_t *buffer) {
  // Serial.print(buffer[0]);
  // Serial.print("C, ");
  // Serial.print(((float)buffer[1]*256.+(float)buffer[2])/100.0);
  // Serial.print("V, ");
  // Serial.print(((float)buffer[3]*256.+(float)buffer[4])/100.0);
  // Serial.print("A, ");
  // Serial.print(((float)buffer[5]*256.+(float)buffer[6]));
  // Serial.print("mAh, ");
  // Serial.print(((float)buffer[7]*256.+(float)buffer[8])*100.0/14.0);
  // Serial.print("RPM ");
  // if (get_crc8(buffer, 9) == buffer[9]) SEGGER_RTT_WriteString(0, "CRC Pass");
  // else SEGGER_RTT_WriteString(0, "CRC Fail");
}


void spamChannel(int wait, int times, int value) {
  static int telem = 0;
  for (int i=0;i<times; i++) {
    //telem++;
    //if(telem>10) telem = 0;
    //dshot_motors.setChannel(0,value, 0 /*telem==0*/);
    //dshot_motors.display();
    ESC.writeMicroseconds(value);
    delay(wait);

  // if (Serial1.available()>=10) {  // TODO: Fix this terrible, lazy implementation
  //   uint8_t telem_buffer[10];
  //   for (int i=0;i<10;i++) {
  //       telem_buffer[i] = Serial1.read();
  //   }
  //   print_telemetry(telem_buffer);
  // }

  }
}


void setup() {
  
  //dshot_motors.setup();
  //Serial.begin(112500);
  delay(500);
  //spamChannel(1, 10, 20);  // Normal Direction
  //spamChannel(1, 10, 0);
  SEGGER_RTT_printf(0,"Servo Attach: %d\n",  ESC.attach(30,1000,2000));
  if(!ESC.attached()) SEGGER_RTT_printf(0,"Servo not attached!\n");
  else SEGGER_RTT_printf(0,"Servo attached!\n");
  ESC.writeMicroseconds(1000);
  delay(1000);
  ESC.writeMicroseconds(2000);
  delay(1000);
  ESC.writeMicroseconds(1500);
  delay(1000);

  SEGGER_RTT_printf(0,"Servo attached!\n");

  pinMode(LED_BUILTIN, OUTPUT);

  //ble_setup(&dshot_motors);

  // KISS telemetry
  // https://www.rcgroups.com/forums/showatt.php?attachmentid=8524039&d=1450424877
  //Serial1.begin(112500);
}
// cmds: https://bitbucket.software-quality-lab.com/projects/EIS/repos/cleanflight/browse/src/main/drivers/pwm_output.h?at=f2468fb894129eefad9ae9c53e4bbb67b15015ed
// https://github.com/bitdump/BLHeli/blob/master/BLHeli_32%20ARM/BLHeli_32%20Firmware%20specs/Digital_Cmd_Spec.txt
void loop() {
  //delay(10);
  //ble_loop();
  //dshot_motors.display();
  // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(800);                       // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  // delay(200);                       // wait for a second
  // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(500);                       // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  // delay(1000);                       // wait for a second

  //SEGGER_RTT_WriteString(0, "Hello World!\n");
  //SEGGER_RTT_WriteString(0, "FW\n");
  //spamChannel(1, 10, 20);  // Normal Direction
  SEGGER_RTT_WriteString(0, "FW 0\n");
  ESC.writeMicroseconds(1000);
  delay(500);
  SEGGER_RTT_WriteString(0, "FW 50\n");
  ESC.writeMicroseconds(1500);
  delay(500);
  SEGGER_RTT_WriteString(0, "FW 100\n");
  ESC.writeMicroseconds(2000);
  delay(500);
  //delay(20);
  
  // SEGGER_RTT_WriteString(0, "Rev Stop");
  // spamChannel(1, 10, 21);  // Reverse Direction
  // SEGGER_RTT_WriteString(0, "Rev Stop");
  // spamChannel(1, 1000, 48);
  // SEGGER_RTT_WriteString(0, "FW GO");
  // spamChannel(1, 2000, 2048);
  //send data only when you receive data:
  
}

/*
64
0x0011 = 0000000000010001
20 20 20 20 20 20 20 20 20 20 20 40 20 20 20 40
0xFFEE = 1111111111101110
40 40 40 40 40 40 40 40 40 40 40 20 40 40 40 20
*/