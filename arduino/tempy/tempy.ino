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

// TODO:
// - [X] Get display working
// - [X] Get temp sense working
// - [X] Display temp C
// - [X] Do division to display temp F
// - [ ] New problem! Wildly floating values!
//   - [ ] Try stabilizing circuit (bad connection?)
//   - [ ] Try different sensor (bad device?)
//   - [ ] Try averaging (take last 10 readings, throw out hi/lo, average rest)
// - [ ] Port to Photon?
// - [ ] Port to Digispark?

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_7segment matrix = Adafruit_7segment();

void setup() {
#ifndef __AVR_ATtiny85__
    Serial.begin(9600);
    Serial.println("7 Segment Backpack Test");
#endif
    matrix.begin(0x70);

    pinMode(A0, INPUT);
    // analogReference(EXTERNAL);
}

void loop() {
    unsigned int tempSense = analogRead(A0);
    // for analogRef 3.3V
    // unsigned int tempC = (tempSense * 330)/1024; // 3.3V = 0.24, * 100 = 24C, so 330
    unsigned int tempC = (tempSense * 500)/1024;
    unsigned int tempF = 32 + (tempC * 9) / 5;
    Serial.print("Temp Sensor: ");
    Serial.print(tempSense);
    Serial.print(", TempC: ");
    Serial.print(tempC);
    Serial.print(", TempF: ");
    Serial.print(tempF);
    Serial.println();


    matrix.print(tempF);
    matrix.writeDisplay();
    delay(200);

    // matrix.writeDigitRaw(0, 0b01111000); // t
    // matrix.writeDigitRaw(1, 0b00011100); // u
    // digit 2 is the colon
    // matrix.writeDigitRaw(3, 0b01010000); // r
    // matrix.writeDigitRaw(4, 0b01011110); // d... because I'm 12. Shut up.
    // matrix.writeDisplay();
}
