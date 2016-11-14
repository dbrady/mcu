// My first REALLY REAL arduino project, yay! lol! Hey, if a
// thermometer doesn't impress you, then maybe this isn't about
// you. :-) We all gotta start somewhere and for me, here is there.
//
// For this project I am using an Arduino UNO to drive a clock display
// mounted on an HT16k33 over I2C, and reading a temperature sensor on
// A0. I have a temp sensor from my arduino starter kit, but I also
// have an LM78 precision thermal voltage regulator that I want to see
// if I can suss out. It likes to sit across 4v-30v and emits
// regulated voltage at 10mV per degree C. At 100C it emits 1V, etc.

// Not sure how I'm going to handle this if I drive it from a Photon
// at 3V. (Answer: probably buy a different thermal regulator, they're
// a buck or two a pop!)

// WOW. This sensor is NOISY as all GET OUT. The Arduino itself is
// pretty wobbly, wobbling +/-5% reading a potentiometer, but the LM35
// sensore is CRAY CRAY. Running on my desk right now, 100 samples
// averaging 43.0 have min/max sensor readings of 32/56. That's
// 60.1F/81.2F, so some smoothing out is still definitely in
// order. (The temp sensor could also be old and sloppy, but reads
// just fine on a slow voltmeter, so there you go.)

// TODO:
// - [X] Figure out dimming! -> matrix.setBrightness(byte) 0..15. 0 is
// dimmest--BUT NOT OFF.

// - [ ] Add a data cleanup mode (read 10, drop min and max)

// - [ ] Figure out how to write the temp scale (probably need to
//       write raw bits, because I want to turn on the decimal AND
//       have it put the "C" or "F" on there--though alternately I
//       could jam in little LEDs for each of the output modes)

// - [ ] Add pushbutton to switch modes:
//       C - Scientific               eg. " 20.6C"
//       F - Burmese/Liberian         eg. " 69.5F"
//       S - ADC Data                 eg. " 43.4A"
//       Data Rate: 1/min 1Hz 2Hz 20Hz 200Hz
//          -> d1 d2 d20 d200 d0.02
//       Sample Size: 1 10 100 500
//          -> S  1 S 10 S100 S500
//          Note: that The UNO doesn't have enough ram for 1000
//              samples.
//          The Photon could probably handle 50k.
//          The Digispark literally may not have ANY. Eeep.

// until enough samples acquired: show rXXX where XXX is number of
// samples that have been read. r001, r002, r003 etc.

// 8888
// (*) (*)
//
// ( ) Scientific Units
// ( ) Burmese/Liberian Units
// ( ) ADC Sensor Units
// ( ) Set Data Rate
// ( ) Set Sample Size
// ( ) Set Brightness

// Oooh, toggles? Raw/Average
// LOL, and data rate could be a potentiometer


#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_7segment matrix = Adafruit_7segment();

// This sensor is a noisy little effer. I want to throw away the
// highest and lowest readings, then average the remaining median
// values. Say I want to average 50 "cleaned" readings, and those
// cleaned readings are 10 readings with the highest and lowest 3
// readings taken away. To keep myself from tripping up, we're going
// to take 10+6 readings, THEN throw away the 6 outliers. That leaves
// us with:
const int SAMPLES=50; // total number of culled readings to average
const int BATCH_SIZE=10; // size of batch AFTER culls taken away
const int CULLS=2; // times to throw away min/max from each batch

// "read":     average reading for this sample after cleaning
// "cleaning": read the samples in batches, and remove min/max
//             values to try to reduce spurious noise
typedef enum {
    SCIENTIFIC_UNITS,
    BURMESE_UNITS,
    ADC_UNITS,
    DISPLAY_READ_ERROR_RATE, // Show best max - worst min for this read
    DISPLAY_TRANSIENT_ERROR_RATE, // Difference between sequential displays
    DISPLAY_DRIFT_RATE, // Difference over a longer term (how long? No
                        // RTC on this baby. Could probably eyeball it
                        // with millis() over an hour at a time tho.)
    DISPLAY_MAX, // show the highest reading from the sample
    DISPLAY_MIN, // show the lowest reading from the sample

    DISPLAY_DAILY_HIGH_READ, // show

    SET_DATA_RATE, // how often to update the display (work in
                   // progress, nothing like realtime, especially now
                   // that we're throwing delays inside the read loop)
                   // (TODO: This could be tightened up by ending each
                   // loop with a framerate-dependent delay, e.g. if
                   // data rate is 5Hz (200ms per frame) and we spent
                   // 162ms farting around getting data, we would say
                   // delay(38), etc.

    SET_BRIGHTNESS,
    SET_SAMPLE_SIZE, // total number of reads before we sign off on a
                     // batch. Each read may have up to BATCH_SIZE-1
                     // extra reads in it
    SET_BATCH_SIZE, // number of reads in a batch before culling
    SET_CULLS, // how many extra pairs to add to each batch, then
               // remove the highest and lowest values from each batch
    TURD, // display a rude word
} MODES;

MODES mode = BURMESE_UNITS;

const uint8_t digits[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01100111, // 9
};

const uint8_t DP = 0b10000000; // .

const uint8_t  C = 0b00111001; // C
const uint8_t  F = 0b01110001; // F
const uint8_t  A = 0b01110111; // A
const uint8_t  D = 0b01011110; // d
const uint8_t  S = 0b01101101; // S
const uint8_t  B = 0b01111100; // b

void displayRudeWord(void) {
    // write rude word
    matrix.writeDigitRaw(0, 0b01111000); // t
    matrix.writeDigitRaw(1, 0b00011100); // u
    // digit 2 is the colon
    matrix.writeDigitRaw(3, 0b01010000); // r
    matrix.writeDigitRaw(4, 0b01011110); // d... because I'm 12. Shut
                                         // up.
    matrix.writeDisplay();
}

void setup() {
#ifndef __AVR_ATtiny85__
    // Serial.begin(9600);
    // Serial.println("7 Segment Backpack Test");
#endif
    matrix.begin(0x70);

    displayRudeWord();
    // analogReference(EXTERNAL);
    matrix.writeDisplay();
    for(int i=0; i<3; i++) {
        for(int j=0; j<16; j++) {
            matrix.setBrightness(j);
            delay(10);
        }
        for(int j=15; j>=0; j--) {
            matrix.setBrightness(j);
            delay(10);
        }
    }
    matrix.setBrightness(15);
    delay(500);
}

void loop() {
    // take a batch of ten, discard lowest/highest, keep rest.
    unsigned int sample_count = 0;
    unsigned int sum = 0;
    unsigned int min = 0;
    unsigned int max = 0;
    unsigned int sample = 0;
    while (sample_count < SAMPLES) {
        for(int i=0; i<BATCH_SIZE + (CULLS*2); i++) {
            sample = analogRead(A0);
            sum += sample;
        }
        for (int i=0; i<CULLS; i++) {
            if (i==0) {
                min = max = sample;
            } else {
                if (min < sample) { min = sample; }
                if (max > sample) { max = sample; }
            }
            sum -= (min + max);
        }
        sample_count += BATCH_SIZE;
        // delaying between reads DOES seem to help stabilize things.
        delay(1);
    }

    // TODO: figure out how to int2float in Arduino or C++
    float average = ((float)sum)/sample_count;
    float tempC = (average * 500)/1024;
    float tempF = 32 + (tempC * 9) / 5;
    // Serial.print("Temp Sensor: ");
    // Serial.print(sample);
    // Serial.print(", Min: ");
    // Serial.print(min);
    // Serial.print(", Max: ");
    // Serial.print(max);
    // Serial.print(", TempC: ");
    // Serial.print(tempC);
    // Serial.print(", TempF: ");
    // Serial.print(tempF);
    // Serial.println();

    // write display
    // matrix.print(tempC);

    // Okay, let's write this display in C here.
    unsigned int value;
    switch(mode) {
    case SCIENTIFIC_UNITS:
        mode = BURMESE_UNITS;
        value = tempC * 10;
        if (value >= 100) { // over 10C?
            matrix.writeDigitRaw(0, digits[value / 100]);
            value = value % 100;
        } else {
            matrix.writeDigitRaw(0, 0);
        }

        // the tens value is the degrees
        matrix.writeDigitRaw(1, DP | digits[value / 10]);
        // the ones values is the tenths
        matrix.writeDigitRaw(3, digits[value % 10]);
        matrix.writeDigitRaw(4, C);
        break;

    case BURMESE_UNITS:
        mode = TURD;
        value = tempF * 10;
        // if temp over 100F... *shrug*
        if (tempF >= 1000) { tempF -= 1000; }

        if (value >= 100) { // over 10C?
            matrix.writeDigitRaw(0, digits[value / 100]);
            value = value % 100;
        } else {
            matrix.writeDigitRaw(0, 0);
        }

        // the tens value is the degrees
        matrix.writeDigitRaw(1, DP | digits[value / 10]);
        // the ones values is the tenths
        matrix.writeDigitRaw(3, digits[value % 10]);
        matrix.writeDigitRaw(4, F);
        break;

    case TURD:
        mode = SCIENTIFIC_UNITS;
        displayRudeWord();
        break;
   }

    // write rude word
    // matrix.writeDigitRaw(0, 0b01111000); // t
    // matrix.writeDigitRaw(1, 0b00011100); // u
    // digit 2 is the colon
    // matrix.writeDigitRaw(3, 0b01010000); // r
    // matrix.writeDigitRaw(4, 0b01011110); // d... because I'm 12. Shut up.

    matrix.writeDisplay();
    delay(2000);
}
