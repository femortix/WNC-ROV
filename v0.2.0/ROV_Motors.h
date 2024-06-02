#define A_INPUT_1 2
#define A_POWER 3 // PWM necessary. Relates to speed of motor A
#define A_INPUT_2 4
#define STANDBY 5 // Prevents or allows connected motors to receive current
#define B_POWER 6 // PWM necessary. Relates to speed of motor B
#define B_INPUT_1 7
#define B_INPUT_2 8

#define CLOCKWISE 0
#define COUNTER_CLOCKWISE 1
#define RIGHT 0
#define LEFT 1

void initializeMotors() {
  pinMode(A_POWER, OUTPUT);
  pinMode(A_INPUT_1, OUTPUT);
  pinMode(A_INPUT_2, OUTPUT);
  pinMode(B_POWER, OUTPUT);
  pinMode(B_INPUT_1, OUTPUT);
  pinMode(B_INPUT_2, OUTPUT);
  pinMode(STANDBY, OUTPUT);

  digitalWrite(STANDBY, LOW);
}

void move(bool motor, bool direction, uint8_t speed) {
  digitalWrite(STANDBY, HIGH);
  // LEFT motor refers to motor A, RIGHT refers to motor B, arbitrarily
  if (motor) {
    // Set inputs 1 and 2 to high and low respectively when counter-clockwise, flip polarity if clockwise
    digitalWrite(A_INPUT_1, direction);
    digitalWrite(A_INPUT_2, !direction);
    analogWrite(A_POWER, speed);
  }
  else {
    digitalWrite(B_INPUT_1, direction);
    digitalWrite(B_INPUT_2, !direction);
    analogWrite(B_POWER, speed);
  }
}

void stop() {
  digitalWrite(STANDBY, LOW);
}