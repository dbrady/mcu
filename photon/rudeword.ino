// rudeword.ino
//
// Well of COURSE the first thing I did is make it say a dirty word. Then I made
// it animate a butterfly, and then I came back and started on proper font
// rendering here.
//
// See butterfly.ino for full details of the wiring. tl;dr D0-D7, A0-A5 drive
// the LED segments and WKP, RX, TX are the digit selectors.

#include "14seg_font.h"

int SEGMENTS[] = { D0, D1, D2, D3, D4, D5, D6, D7, /* A-G2 */
                   A0, A1, A2, A3, A4, A5, A6 };
int NUM_SEGMENTS = sizeof(SEGMENTS) / sizeof(SEGMENTS[0]);

int DIGITS[] = { WKP, RX, TX };
int NUM_DIGITS = 6;
int NUM_DIGIT_SELECTORS = sizeof(DIGITS) / sizeof(DIGITS[0]);

int ON=HIGH; // power on a segment to turn it on...
int OFF=LOW;
int DIGIT_ON=OFF; // ...and ground the desired digit to complete the circuit
int DIGIT_OFF=ON;

// This is the display memory
static char *display = "DBrady";

void clear(void) {
  for(int i=0; i<NUM_SEGMENTS; i++) {
    digitalWrite(SEGMENTS[i], OFF);
  }
  for(int i=0; i<NUM_DIGIT_SELECTORS; i++) {
    digitalWrite(DIGITS[i], DIGIT_OFF);
  }
}

void selectDigit(int digit) {
  digitalWrite(DIGITS[0], ((digit >> 0) & 1) ? DIGIT_ON : DIGIT_OFF );
  digitalWrite(DIGITS[1], ((digit >> 1) & 1) ? DIGIT_ON : DIGIT_OFF );
  digitalWrite(DIGITS[2], ((digit >> 2) & 1) ? DIGIT_ON : DIGIT_OFF );
}

void writeDigit(uint16_t digit) {
  for(int i=0; i<NUM_SEGMENTS; i++) {
    digitalWrite(SEGMENTS[i], ((digit>>i)&1) ? ON : OFF );
  }
}

void writeDisplay(void) {
  for(int i=0; i<NUM_DIGITS; i++) {
    selectDigit(i+1);
    writeDigit(alphafonttable[display[i]]);
    delay(1);
  }
}

// tell me you're alive, baby
void winkOn(void) {
  digitalWrite(D7, HIGH);
}

// okay baby, go back to playing dead
void winkOff(void) {
  digitalWrite(D7, LOW);
}

void setup() {
  // turn off column selects by setting them high (common anode)
  for(int i=0; i<NUM_DIGIT_SELECTORS; i++) {
    pinMode(DIGITS[i], OUTPUT);
    digitalWrite(DIGITS[i], DIGIT_OFF);
  }
  // turn off segments by setting them low
  for(int i=0; i<NUM_SEGMENTS; i++) {
    pinMode(SEGMENTS[i], OUTPUT);
    digitalWrite(SEGMENTS[i], OFF);
  }

  // did we even boot?!? Get some attention!
  for(int i=0; i<2; i++) {
    for(int j=0; j<NUM_SEGMENTS; j++) {
      digitalWrite(SEGMENTS[j], ON);
    }
    for(int j=0; j<NUM_DIGITS; j++) {
      selectDigit(j+1);
      delay(25);
    }
    for(int j=NUM_DIGITS; j>0; j--) {
      selectDigit(j+1);
      delay(25);
    }
  }
  clear();
  delay(200);
  for(int i=0; i<200; i++) {
    for(int j=0; j<NUM_SEGMENTS; j++) {
      digitalWrite(SEGMENTS[j], ON);
    }
    for(int j=0; j<NUM_DIGITS; j++) {
      selectDigit(j+1);
      delay(1);
    }
  }
  clear();
  delay(200);
}

void loop() {
  writeDisplay();
}
