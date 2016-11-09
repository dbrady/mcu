// still only 7 segments lol
int DIGITS[6] = {A0, A1, A2, A3, A4, A5};
int A=0;
int B=1;
int C=2;
int D=3;
int E=4;
int F=5;
int G=6;
int DP=7;

int SEGMENTS[8] = {D0, D1, D2, D3, D4, D5, D6, D7};

int ON=HIGH; // power on a segment to turn it on...
int OFF=LOW;
int DIGIT_ON=OFF; // ...and ground the desired digit to complete the circuit
int DIGIT_OFF=ON;

int SEQUENCE[8] = {A, B, G, E, D, C, G, F};

void clear(void) {
  for(int i=0; i<8; i++) {
    digitalWrite(SEGMENTS[i], OFF);
  }
  for(int i=0; i<6; i++) {
    digitalWrite(DIGITS[i], DIGIT_OFF);
  }
}

void allOn(void) {
  for(int i=0; i<8; i++) {
    digitalWrite(SEGMENTS[i], ON);
  }
  for(int i=0; i<6; i++) {
    digitalWrite(DIGITS[i], DIGIT_ON);
  }
}

void setup() {
  // turn off column selects by setting them high (common anode)
  for(int i=0; i<6; i++) {
    pinMode(DIGITS[i], OUTPUT);
    digitalWrite(DIGITS[i], DIGIT_OFF);
  }
  // turn off segments by setting them low
  for(int i=0; i<8; i++) {
    pinMode(SEGMENTS[i], OUTPUT);
    digitalWrite(SEGMENTS[i], OFF);
  }

  // get some attention!
  for(int i=0; i<10; i++) {
    clear();
    delay(250);
    allOn();
    delay(250);
  }
  clear();
}

uint8_t font7[] = {

}

void loop() {
  // int bits[6] = {0, 0b01011000, 0b00011100, 0b01010100, 0b01111000, 0,};

  // " nErd "
  // int bits[6] = {0b00000000, 0b01010100, 0b01111001, 0b01010000, 0b01011110, 0b00000000,};

  int bits[6] = {
    0b01110001,
    0b00111110,
    0b00111001,
    0b11110000,
    0b01111001,
    0b11110011,
  };

  int scans=0;

  for (int j=0; j<6; j++) {
    clear();
    digitalWrite(DIGITS[(j-1)%6], DIGIT_OFF);
    digitalWrite(DIGITS[j%6], DIGIT_ON);
    for (int i=0; i<8; i++) {
      digitalWrite(SEGMENTS[i], ((bits[j] >> i) & 1) ? ON : OFF);
    }
    delay(1);
  }
}
