#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "dshot_nrf52.h"

static nrfx_pwm_t nordic_nrf5_pwm_instance[] = {
    NRFX_PWM_INSTANCE(0)  // there are 4 of these (0-3) on the NRF52840
};

#define DSHOT_0 19
#define DSHOT_1 39

void DShotPWMOutput::setup(uint8_t m1_pin, bool m1_dir, 
                           uint8_t m2_pin, bool m2_dir, 
                           uint8_t m3_pin, bool m3_dir, 
                           uint8_t m4_pin, bool m4_dir) {  // todo: make this more configurable
    config = NRFX_PWM_DEFAULT_CONFIG(
        (uint8_t)digitalPinToPinName(m1_pin),
        (uint8_t)digitalPinToPinName(m2_pin),
        (uint8_t)digitalPinToPinName(m3_pin),
        (uint8_t)digitalPinToPinName(m4_pin)
    );
    motor_directions[0] = m1_dir;
    motor_directions[1] = m2_dir;
    motor_directions[2] = m3_dir;
    motor_directions[3] = m4_dir;

    // https://blck.mn/2016/11/dshot-the-new-kid-on-the-block/
    // dshot 300 timing requires 3.33us per bit
    // 0 = 1.25us (37.4251497% duty) = 20
    // 1 = 2.5us (74.850299401% duty) = 40
    config.top_value    = 52;  // 16 bit value, max 65536/2-1. 0-52 count is 0.6% fast, 0-53 is 1.2% slow

    // dshot 150 timing requires 6.66us per bit
    // 0 = 2.5us (37.4251497% duty) = 40
    // 1 = 5us (74.850299401% duty) = 80
    //config.top_value    = 106;  // 16 bit value, max 65536/2-1. 0-106 count is 0.3% fast, 0-105 is 1% slow

    // prescaler can set the base clock 250kHz to 16Mhz (by powers of 2)
    // COUNTERTOP resets it every N clocks
    
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
uint16_t DShotPWMOutput::createPacket(uint16_t throttle, bool telemetry_request){
uint8_t csum = 0;
throttle <<= 1;
// Previously, Indicate as command if less than 48 with a telemetry request. Unsure why that was done.
// 
if (telemetry_request || throttle < 48)
    throttle |= 1;  // TODO: The above is wrong, 1 here indicates telemetry request :s
uint16_t csum_data = throttle;
for (byte i=0; i<3; i++){
    csum ^= csum_data;
    csum_data >>= 4;
}
csum &= 0xf;
return (throttle<<4)|csum;
}

void DShotPWMOutput::setChannel(int channel, uint16_t throttle, bool telemetry_request) {
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
            if (packet & mask) seq_values[i].channel_0 = (1 << 15) | DSHOT_1;
            else seq_values[i].channel_0 = (1 << 15) | DSHOT_0;
            mask >>= 1;
        }
        break;
        case 1:
        for(int i=0;i<16;i++) {
            if (packet & mask) seq_values[i].channel_1 = (1 << 15) | DSHOT_1;
            else seq_values[i].channel_1 = (1 << 15) | DSHOT_0;
            mask >>= 1;
        }
        break;
        case 2:
        for(int i=0;i<16;i++) {
            if (packet & mask) seq_values[i].channel_2 = (1 << 15) | DSHOT_1;
            else seq_values[i].channel_2 = (1 << 15) | DSHOT_0;
            mask >>= 1;
        }
        break;
        case 3:
        for(int i=0;i<16;i++) {
            if (packet & mask) seq_values[i].channel_3 = (1 << 15) | DSHOT_1;
            else seq_values[i].channel_3 = (1 << 15) | DSHOT_0;
            mask >>= 1;
        }
        break;
        default:
        // some sort of fault would be nice
        break;
    }
    // Work around an apparent bug in the PWM output, by forcing a zero cycle at the end
    // notes: https://devzone.nordicsemi.com/f/nordic-q-a/46269/nrf52-spike-after-last-period-with-inverted-polarity-pwm
    seq_values[16].channel_0 = (1 << 15);
    seq_values[16].channel_1 = (1 << 15);
    seq_values[16].channel_2 = (1 << 15);
    seq_values[16].channel_3 = (1 << 15);
    // for (int i=0;i<16; i++) {
    //   Serial.print(seq_values[i].channel_0); Serial.print(" ");
    // }
    // Serial.println("");

}

void DShotPWMOutput::setThrottle(int channel, int8_t throttle, bool telemetry_request)
{
  uint16_t output = 0;

  // enforce blanking on zero crossing
  
  // if we're changing direction
  if ((last_motor_command[channel]<0 && throttle>0) || (last_motor_command[channel]>0 && throttle<0)) {
    // Skip moving until zero_blanking_ms has elapsed
    if (millis() - last_motor_time[channel] < zero_blanking_ms) {
        setChannel(channel, 0, telemetry_request);
        return;   // skip other actions
    }
  }
  // otherwise store the desired command
  last_motor_command[channel] = throttle;

  if (motor_directions[channel] == 1) throttle = -throttle; // invert if needed
  if (throttle == 0) {
    output = 0;
  } else if (throttle<0) {
    throttle = -throttle;
    output = min(2047, 1048+((uint16_t)throttle)*10);
    last_motor_time[channel] = millis();
  } else {
    output = min(1047, 48+((uint16_t)throttle)*10);
    last_motor_time[channel] = millis();
  }
  
  setChannel(channel, output, telemetry_request);
}

void DShotPWMOutput::display(){
    sequence.values.p_individual = &seq_values[0];
    sequence.length = NRF_PWM_VALUES_LENGTH(seq_values);
    sequence.repeats = 0;
    sequence.end_delay = 0;

    // todo: check for errors?
    // NRFX_PWM_FLAG_STOP, NRFX_PWM_FLAG_LOOP
    (void)nrfx_pwm_simple_playback(&nordic_nrf5_pwm_instance[0], &sequence, 1, NRFX_PWM_FLAG_STOP );

}

