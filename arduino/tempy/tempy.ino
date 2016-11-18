// My first REALLY REAL arduino project, yay! lol! Hey, if a
// thermometer doesn't impress you, then maybe this isn't about
// you. :-) We all gotta start somewhere and for me, here is there.
//
// For this project I am using an Arduino UNO to drive a clock display
// mounted on an HT16k33 over I2C, and reading a temperature sensor on
// A0. I have a temp sensor from my arduino starter kit, but I also
// have an LM35 precision thermal voltage regulator that I want to see
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

// MUCH RESEARCH, SUCH ANALOG, VERY WOW: Turns out the LM35 and TMP36 sensors
// are *analog*, and even a 10cm jumper wire is enough of an antenna to pick up
// shedloads of noise. I switched to a DS18B20 digital temperature sensor that
// communicates via the OneWire protocol. Temps immediately became stable and
// rock-solid. YAY! WIN! Using an analog sensor can probably still be made to
// work but I would want to shield the ever living crap out of the connector:
// shorten the leads (ideally direct-connect to arduino), shielding around BOTH
// the ground and analog wires, buffer caps and pull-up resistors at both ends,
// etc.

// TODO:
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
//
/// - [ ] Add a brightness toggle button

// 8888
// (O) Toggle Scientific/Burmese/Rudeword/Cycle
// (O) Toggle Brightness


#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <OneWire.h>
#include <DallasTemperature.h>


// Data wire is plugged into port 2 on the Arduino (D2)
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

Adafruit_7segment matrix = Adafruit_7segment();

typedef enum {
    SCIENTIFIC_UNITS,
    BURMESE_UNITS,
    SET_BRIGHTNESS,
    TURD, // display a rude word
} MODES;

MODES mode = BURMESE_UNITS;

const uint8_t digits[37] = {
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
    0b00000000, // <SPC>
    0b01110111, // A
    0b01111100, // b
    0b00111001, // C
    0b01011110, // d
    0b01111001, // E
    0b01110001, // F
    0b00111101, // G
    0b01110100, // h
    0b00000110, // I
    0b00011110, // J
    0b01010011, // ? K
    0b00111000, // L
    0b01010011, // ? M
    0b01010100, // n
    0b01011100, // o
    0b01110011, // P
    0b01010011, // ? Q
    0b01010000, // r
    0b01101101, // S
    0b01111000, // t
    0b00011100, // u
    0b01010011, // ? V
    0b01010011, // ? W
    0b01010011, // ? X
    0b01101110, // y
    0b01010011, // ? Z
};

const uint8_t DP = 0b10000000; // .
const uint8_t  B = 0b01111100; // b


void displayWord(char *word) {
  int idx;
  uint8_t ch;

  int i,j;
  for (i=0, j=0; i<4; i++,j++) {
    if (i==2) { j++; }
    ch = word[i];
    if (ch == ' ') {
      idx = 10;
    } else if (ch >= '0' && ch <= '9') {
      idx = ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
      idx = ch - 'a' + 11;
    }
    matrix.writeDigitRaw(j, digits[idx]);
  }
  matrix.writeDisplay();
}

void displayRudeWord(void) {
  char *tourettes[] = {
    "fart",
    "turd",
    "boob",
    "shat",
    "tits",
    "poop",
    "crap",
    "dolt",
  };

  displayWord(tourettes[random(sizeof(tourettes)/sizeof(tourettes[0]))]);
}

void clear(void) {
  for(int i=0; i<5; i++) {
    matrix.writeDigitRaw(i, 0);
  }
  matrix.writeDisplay();
}

void setup() {
  // TODO: make Serial begin/print/println macros for Debug vs ATtiny85 vs Release
#ifndef __AVR_ATtiny85__
  Serial.begin(9600);
  Serial.println("tempy - THERMOMETER OF DOOOOOM");
#endif
  sensors.begin();
  matrix.begin(0x70);

  // analogReference(EXTERNAL);
  matrix.writeDisplay();
  for(int i=0; i<3; i++) {
    displayRudeWord();
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
  matrix.setBrightness(15);
  mode = SCIENTIFIC_UNITS;
}

void loop() {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  float tempC = sensors.getTempCByIndex(0);
  float tempF = (tempC * 9) / 5 + 32;
  Serial.println(tempC);

  // Okay, let's write this display in C here.
  // TODO: Unsigned? Turns out temps go negative in Utah in the winter...
  unsigned int value;
  clear();
  switch(mode) {
  case SCIENTIFIC_UNITS:
    mode = BURMESE_UNITS;
    Serial.print("Trying to slam out tempC: "); Serial.println(tempC);
    value = tempC * 10;
    Serial.print("  Value: "); Serial.println(value);
    if (value >= 100) { // over 10C?
      Serial.print("  Writing tens: "); Serial.println(value / 100);
      matrix.writeDigitRaw(0, digits[value / 100]);
      value = value % 100;
      Serial.print("  Setting Value to mod 100https://github.com/milesburton/Arduino-Temperature-Control-Library/blob/master/examples/Simple/Simple.pde: "); Serial.println(value);
    } else {
      // matrix.writeDigitRaw(0, 0);
    }

    // the tens value is the degrees
    matrix.writeDigitRaw(1, DP | digits[value / 10]);
    // the ones values is the tenths
    matrix.writeDigitRaw(3, digits[value % 10]);
    matrix.writeDigitRaw(4, digits[13]);
    break;

  case BURMESE_UNITS:
    mode = TURD;
    value = tempF * 10;
    // if temp over 100F... *shrug*
    while (tempF >= 1000) { tempF -= 1000; }

    if (value >= 100) { // over 10C?
      matrix.writeDigitRaw(0, digits[value / 100]);
      value = value % 100;
    } else {
      // matrix.writeDigitRaw(0, 0);
    }

    // the tens value is the degrees
    matrix.writeDigitRaw(1, DP | digits[value / 10]);
    // the ones values is the tenths
    matrix.writeDigitRaw(3, digits[value % 10]);
    matrix.writeDigitRaw(4, digits[16]);
    break;

  case TURD:
    mode = SCIENTIFIC_UNITS;
    displayRudeWord();
    break;
  }

  matrix.writeDisplay();
  delay(1000);
}
