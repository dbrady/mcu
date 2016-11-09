// butterfly.ino

// Result of an evening screwing around with Photon. I'm driving a funky 6-digit
// 14-segment LED display with it. The Photon has 18 GPIO pins; 15 pins send +V
// to a given LED segment (14 segments + 1 for the decimal point). That leaves 3
// pins for selecting the digit... not to worry! Enter the 74LS138, a 3-to-8
// binary decoder. Give it three pins and it will give you 8 in return: whatever
// binary number you encode, it will select that pin.
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
//      |       \  |  /       |
//
// D0
//

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

int display[] = { 36, 47, 87, 109, 125, 126 };



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

void winkOn(void) {
  digitalWrite(D7, HIGH);
}

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

  // get some attention!
  for(int i=0; i<4; i++) {
    for(int j=0; j<NUM_SEGMENTS; j++) {
      digitalWrite(SEGMENTS[j], ON);
    }
    for(int j=0; j<NUM_DIGITS; j++) {
      selectDigit(j+1);
      delay(25);
    }
  }
  clear();
}

void waggle(void) {
  digitalWrite(TX, LOW);
  delay(50);
  digitalWrite(TX, HIGH);
  delay(50);
}


uint16_t butterfly[] = {
  // frame 1:  \ /
  //           / \
  0b010110100000000,

  // frame 2: \|
  //          /|
  0b011001100000000,
};

int pos=0;

void loop() {
  for(int i=0; i<NUM_DIGITS; i++) {
    pos = (pos + 1) % NUM_DIGITS;
    // draw butterfly at rest
    selectDigit(pos+1);
    writeDigit(butterfly[0]);
    delay(400);

    // flap!
    writeDigit(butterfly[1]);
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
  }
}
