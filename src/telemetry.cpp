#include <Arduino.h>
// Note: Unused at the moment

// from: KISS_telemetry_protocol.pdf
typedef struct {
   int8_t temperature;    // [0] = Temperature (C)
   uint16_t voltage;      // [1:2] = Voltage*100
   uint16_t current;      // [3:4] = Amps*100
   uint16_t consumption;  // [5:6] = mAh
   uint16_t erpm;         // [7-8] = ERpm/100 = RPM*Poles/100
   uint8_t crc;           // [9] = CRC
} kiss_telemetry;


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

void setup_telemetry() {
   // KISS telemetry
  // https://www.rcgroups.com/forums/showatt.php?attachmentid=8524039&d=1450424877
  //Serial1.begin(112500);
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

void read_telemetry() {
  // if (Serial1.available()>=10) {  // TODO: Fix this terrible, lazy implementation
  //   uint8_t telem_buffer[10];
  //   for (int i=0;i<10;i++) {
  //       telem_buffer[i] = Serial1.read();
  //   }
  //   print_telemetry(telem_buffer);
  // }
}

void motor_debug_test() {
   //SEGGER_RTT_WriteString(0, "Hello World!\n");

  //Direction 1) 48 is the slowest, 1047 is the fastest
  //Direction 2) 1049 is the slowest, 2047 is the fastest
  // 1048 does NOT stop the motor! Use command 0 for that.
    
  // SEGGER_RTT_WriteString(0, "Rev n\n");
  // spamChannel(1, 2000, 2047);
  // SEGGER_RTT_WriteString(0, "Rev mid\n");
  // spamChannel(1, 2000, 0);
  // SEGGER_RTT_WriteString(0, "FW GO\n");
  // spamChannel(1, 2000, 1047);
  // SEGGER_RTT_WriteString(0, "FW stop\n");
  // spamChannel(1, 2000, 0);

}