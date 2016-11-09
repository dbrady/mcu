// butterfly.ino
//
// Result of an evening screwing around with Photon. I'm driving a funky 6-digit
// 14-segment LED display with it, and rather than make it spell a rude word
// because I'm twelve (not saying I'm better than that; I've already done it and
// moved on). This project animates a little butterfly flapping across the
// display.
//
// The LED is common anode; you put +V to one of the segments (the rows) and
// ground one of the digits (the columns) to light up one segment in one
// digit. The 74LS138 is cleverly designed to be "active low", so no extra parts
// are needed to make it work. Ground pin A and leave pins B and C high, and it
// will ground digit 1 of the display. Ground pins A and C, and it will ground
// digit 5 of the display. Nifty.
//
// The Photon has 18 GPIO pins; 15 pins send +V to a given LED segment (14
// segments + 1 for the decimal point). That leaves 3 pins for selecting the
// digit... not to worry! Enter the 74LS138, a 3-to-8 binary decoder. Give it
// three pins and it will give you 8 in return: whatever binary number you
// encode, it will select that pin.
//
// One drawback to this design is that all of my pins are in use! If I wanted to
// make a sensing/display project, I'd need to sacrifice the decimal point to
// free up that pin, or give up 2 of the digits (4 digits can be driven with 2
// pins).
//
// Q. Why not just use I2C?
// A. Because shut up.

// Wireup:
//
// 14-Segment LED:
//
//                 a
//       ---------------------
//      | \        |        / |
//      |  \       |       /  |
//      |   \      |      /   |
//    f |  h \   j |   k /    | b
//      |     \    |    /     |
//      |      \   |   /      |
//      |  g1   \  |  /   g2  |
//       ---------- ----------
//      |  g1   /  |  \   g2  |
//      |      /   |   \      |
//      |     /    |    \     |
//    f |  n /   m |   l \    | b
//      |   /      |      \   |
//      |  /       |       \  |
//      | /        |        \ |
//       ---------------------
//                 d
//
// D0 -> A
// D1 -> B
// D2 -> C
// D3 -> D
// D4 -> E
// D5 -> F
// D6 -> G1
// D7 -> G2
// A0 -> H
// A1 -> J
// A2 -> K
// A3 -> L
// A4 -> M
// A5 -> N
//
// WKP -> DIGIT_SELECT_A
// RX  -> DIGIT_SELECT_B
// TX  -> DIGIT_SELECT_C
#include "14seg_font.h"

int SEGMENTS[] = { D0, D1, D2, D3, D4, D5, D6, D7, /* A-G2 */
                   A0, A1, A2, A3, A4, A5, A6 };
int NUM_SEGMENTS = sizeof(SEGMENTS) / sizeof(SEGMENTS[0]);

// DIGITS connects to the ABC input of a 74LS138 decoder. It's a binary decoder
// with ACTIVE LOW outputs, which is perfect for driving our little common anode
// LEDs. Treat WKP as the 1-bit, RX as the 2-bit, and RX as the 4-bit, and
// digit 0-7 will be sinked to turn it on. In order to get an "OFF" option, I
// have not wired up Output 0, which is on when DIGITS is set to 000.
int DIGITS[] = { WKP, RX, TX };
int NUM_DIGITS = 6;
int NUM_DIGIT_SELECTORS = sizeof(DIGITS) / sizeof(DIGITS[0]);

int ON=HIGH; // power on a segment to turn it on...
int OFF=LOW;
int DIGIT_ON=OFF; // ...and ground the desired digit to complete the circuit
int DIGIT_OFF=ON;

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

// buffer underrun yourself all you want lol
void writeWord(char *display) {
  for(int i=0; i<NUM_DIGITS; i++) {
    selectDigit(i+1);
    writeDigit(alphafonttable[display[i]]);
    delay(1);
    clear();
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
  for(int i=0; i<500; i++) {
    writeWord("Butt  ");
    delay(1);
  }
  for(int i=0; i<500; i++) {
    writeWord(" erfly");
    delay(1);
  }
  clear();
  delay(500);
}

uint16_t butterfly[] = {
  // frame 1:  \ / .
  //           / \ .
  0b010110100000000,

  // frame 2: \|  .
  //          /|  .
  0b011001100000000,
};

int pos=0;

// #define LOCKSTEP_MODE 1

void loop() {
  for(int i=0; i<NUM_DIGITS; i++) {
    pos = (pos + 1) % NUM_DIGITS;
    // draw butterfly at rest
    selectDigit(pos+1);
    writeDigit(butterfly[0]);
#ifdef LOCKSTEP_MODE
    delay(200);
#else
    delay(400);
#endif

    // flap!
    writeDigit(butterfly[1]);
#ifdef LOCKSTEP_MODE
    delay(50);
    selectDigit(pos+2);
    delay(50);
#else
    delay(15);

    // gliiiiide random distance
    int j=random(3) + random(3) + random(3) + random(3);
    for(int k=0; k<j; k++) {
      // gliiiiiide
      pos = (pos + 1) % NUM_DIGITS;
      selectDigit(pos+1);
      delay(15+k*30);
    }
    delay(200);
#endif
  }
}
