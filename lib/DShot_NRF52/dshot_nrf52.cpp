#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "dshot_nrf52.h"

static nrfx_pwm_t nordic_nrf5_pwm_instance[] = {
    NRFX_PWM_INSTANCE(0)  // there are 4 of these (0-3) on the NRF52840
};


void DShotPWMOutput::setup() {  // todo: make this more configurable
config = NRFX_PWM_DEFAULT_CONFIG(
    (uint8_t)digitalPinToPinName(30),  // 30 = M1
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
uint16_t DShotPWMOutput::createPacket(uint16_t throttle, bool telemetry_request){
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

void DShotPWMOutput::display(){
    sequence.values.p_individual = &seq_values[0];
    sequence.length = NRF_PWM_VALUES_LENGTH(seq_values);
    sequence.repeats = 0;
    sequence.end_delay = 0;

    // todo: check for errors?
    // NRFX_PWM_FLAG_STOP, NRFX_PWM_FLAG_LOOP
    (void)nrfx_pwm_simple_playback(&nordic_nrf5_pwm_instance[0], &sequence, 1, NRFX_PWM_FLAG_STOP );

}

