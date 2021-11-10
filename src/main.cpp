#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <nrfx.h>
#include <nrfx_pwm.h>
#include "SEGGER_RTT.h"
// note: PWM needs to be enabled in nrfx_config.h

// 16 values required for 
static nrf_pwm_values_individual_t seq_values[16];

static nrfx_pwm_t nordic_nrf5_pwm_instance[] = {
    NRFX_PWM_INSTANCE(0)  // there are 4 of these (0-3) on the NRF52840
};

// TODO: Migrate NRF5 PWM -> dshot into it's own library or at least header+src files

// from: KISS_telemetry_protocol.pdf
typedef struct {
   int8_t temperature;    // [0] = Temperature (C)
   uint16_t voltage;      // [1:2] = Voltage*100
   uint16_t current;      // [3:4] = Amps*100
   uint16_t consumption;  // [5:6] = mAh
   uint16_t erpm;         // [7-8] = ERpm/100 = RPM*Poles/100
   uint8_t crc;           // [9] = CRC
} kiss_telemetry;

class DShotPWMOutput{
  public:
  nrf_pwm_sequence_t sequence;
  nrfx_pwm_config_t config;

  DShotPWMOutput(){ }

  void setup() {
    config = NRFX_PWM_DEFAULT_CONFIG(
      (uint8_t)digitalPinToPinName(17),
      NRFX_PWM_PIN_NOT_USED,
      NRFX_PWM_PIN_NOT_USED,
      NRFX_PWM_PIN_NOT_USED
      );

    // https://blck.mn/2016/11/dshot-the-new-kid-on-the-block/
    // dshot 300 timing requires 3.33us per bit
    // 0 = 1.25us (37.4251497% duty)
    // 1 = 2.5us (74.850299401% duty)
    // prescaler can set the base clock 250kHz to 16Mhz (by powers of 2)
    // COUNTERTOP resets it every N clocks
    config.top_value    = 52;  // 16 bit value, max 65536/2-1. 0-52 count is 0.6% fast, 0-53 is 1.2% slow
    // frequency: 301.88kHz ~= 300kHz
    // thus: 
    // 0 = 20 count (1.25us)
    // 1 = 40 count (2.5us)

    // "normal" PWM: 2500 count, 1Mhz, set duty cycle to number of ms (~1k-2k)
    // oneshot125: same but 4000 count, 16Mhz

    config.base_clock   = NRF_PWM_CLK_16MHz;
    config.count_mode   = NRF_PWM_MODE_UP;
    
    config.load_mode    = NRF_PWM_LOAD_INDIVIDUAL;
    config.step_mode    = NRF_PWM_STEP_AUTO;  // play the requested number of time

    nrfx_pwm_init(&nordic_nrf5_pwm_instance[0], &config, nullptr, nullptr);
  }

  /*
    Prepare data packet, attach 0 to telemetry bit, and calculate CRC
    throttle: 11-bit data
  */
  // From: https://github.com/gueei/DShot-Arduino/blob/master/src/DShot.cpp
  uint16_t createPacket(uint16_t throttle, bool telemetry_request = false){
    uint8_t csum = 0;
    throttle <<= 1;
    // Previously, Indicate as command if less than 48 with a telemetry request. Unsure why that was done.
    // 
    if (telemetry_request)
      throttle |= 1;  // TODO: The above is wrong, 1 here indicates telemetry request :s
    uint16_t csum_data = throttle;
    for (byte i=0; i<3; i++){
      csum ^= csum_data;
      csum_data >>= 4;
    }
    csum &= 0xf;
    return (throttle<<4)|csum;
  }

  void setChannel(int channel, uint16_t throttle, bool telemetry_request = false) {
    // create packet
    uint16_t packet = createPacket(throttle, telemetry_request);
    //Serial.println(packet, HEX);

    uint16_t mask = 0x8000;
    // todo: use memory offsets to specify channel rather than switch case
    switch(channel) {
      case 0:
        for(int i=0;i<16;i++) {
          // high bit set inverts PWM polarity so that the HIGH portion
          // of the duty cycle begins right at the start of the cycle
          if (packet & mask) seq_values[i].channel_0 = (1 << 15) | 40;
          else seq_values[i].channel_0 = (1 << 15) | 20;
          mask >>= 1;
        }
        break;
      case 1:
        for(int i=0;i<16;i++) {
          if (packet & mask) seq_values[i].channel_1 = (1 << 15) | 40;
          else seq_values[i].channel_1 = (1 << 15) | 20;
          mask >>= 1;
        }
        break;
      case 2:
        for(int i=0;i<16;i++) {
          if (packet & mask) seq_values[i].channel_2 = (1 << 15) | 40;
          else seq_values[i].channel_2 = (1 << 15) | 20;
          mask >>= 1;
        }
        break;
      case 3:
        for(int i=0;i<16;i++) {
          if (packet & mask) seq_values[i].channel_3 = (1 << 15) | 40;
          else seq_values[i].channel_3 = (1 << 15) | 20;
          mask >>= 1;
        }
        break;
      default:
      // some sort of fault would be nice
        break;
    }
    // for (int i=0;i<16; i++) {
    //   Serial.print(seq_values[i].channel_0); Serial.print(" ");
    // }
    // Serial.println("");

  }

  void display(){
      sequence.values.p_individual = &seq_values[0];
      sequence.length = NRF_PWM_VALUES_LENGTH(seq_values);
      sequence.repeats = 0;
      sequence.end_delay = 0;

      // todo: check for errors?
      // NRFX_PWM_FLAG_STOP, NRFX_PWM_FLAG_LOOP
      (void)nrfx_pwm_simple_playback(&nordic_nrf5_pwm_instance[0], &sequence, 1, NRFX_PWM_FLAG_STOP );
    
    }
 };

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
  // if (get_crc8(buffer, 9) == buffer[9]) Serial.println("CRC Pass");
  // else Serial.println("CRC Fail");
}

DShotPWMOutput test;

void spamChannel(int wait, int times, int value) {
  static int telem = 0;
  for (int i=0;i<times; i++) {
    telem++;
    if(telem>10) telem = 0;
    test.setChannel(0,value, telem==0);
    test.display();
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

// some BLE stuff
//#include <SPI.h>
// #include <BLEPeripheral.h>


void setup_ble() {

}


void setup() {
  
  test.setup();
  //Serial.begin(112500);
  delay(500);
  spamChannel(1, 1500, 0);

  pinMode(LED_BUILTIN, OUTPUT);

  // KISS telemetry
  // https://www.rcgroups.com/forums/showatt.php?attachmentid=8524039&d=1450424877
  //Serial1.begin(112500);
}
// cmds: https://bitbucket.software-quality-lab.com/projects/EIS/repos/cleanflight/browse/src/main/drivers/pwm_output.h?at=f2468fb894129eefad9ae9c53e4bbb67b15015ed
// https://github.com/bitdump/BLHeli/blob/master/BLHeli_32%20ARM/BLHeli_32%20Firmware%20specs/Digital_Cmd_Spec.txt
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(800);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second

  SEGGER_RTT_WriteString(0, "Hello World!\n");
  // Serial.println("FW");
  // spamChannel(1, 10, 20);  // Normal Direction
  // Serial.println("FW Stop");
  // spamChannel(1, 1000, 48);
  // Serial.println("FW GO");
  // spamChannel(1, 1000, 90);
  
  // Serial.println("Rev Stop");
  // spamChannel(1, 10, 21);  // Reverse Direction
  // Serial.println("Rev Stop");
  // spamChannel(1, 1000, 48);
  // Serial.println("FW GO");
  // spamChannel(1, 1000, 90);
  // send data only when you receive data:
  
}

/*
64
0x0011 = 0000000000010001
20 20 20 20 20 20 20 20 20 20 20 40 20 20 20 40
0xFFEE = 1111111111101110
40 40 40 40 40 40 40 40 40 40 40 20 40 40 40 20
*/