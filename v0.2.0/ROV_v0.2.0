#include "ROV_Pressure.h" // In-house. Separates pressure sensor code
#include "ROV_Temperature.h" // In-house. Separates temperature probe code
#include "ROV_SDCard.h" // In-house. Separates SD card chip code
#include "ROV_Motors.h" // In-house. Separates motor code
#include "ROV_Accelerometer.h" // In-house. Separates 6-axis accelerometer code

bool running = false;

void setup() {
  Serial.begin(9600);
  Wire.setClock(400000); // MPU 6050 communicates via 400 kHz I2C clock
  Wire.begin();
  setByte(MPU_A, GYRO_CONFIG, 0b00010000); // [-500, 500] deg/s over [-32768, 32767]
  setByte(MPU_A, ACCEL_CONFIG, 0b00000000); // [-2, 2] Gs over [-32768, 32767]
  setByte(MPU_A, XG_OFFSET_H, 0b00000000); // Add 92 bits to GYRO_XOUT
  setByte(MPU_A, XG_OFFSET_L, 0b01011100);
  setByte(MPU_A, YG_OFFSET_H, 0b00000000); // Add 33 bits to GYRO_YOUT
  setByte(MPU_A, YG_OFFSET_L, 0b00100001);
  setByte(MPU_A, ZG_OFFSET_H, 0b11111111); // Subtract 7 bits from GYRO_ZOUT
  setByte(MPU_A, ZG_OFFSET_L, 0b11111001);

  // Below code takes n samples and gets average accelerations at wherever its initial orientation is
  uint16_t n = 1000;
  float acc[3] = {0, 0, 0};
  for (uint16_t i = 0; i < n; i++) {
    getByte(MPU_A, ACCEL_XOUT_H);
    acc[0] += getTwoBytes(MPU_A, ACCEL_XOUT_H) / n;
    getByte(MPU_A, ACCEL_YOUT_H);
    acc[1] += getTwoBytes(MPU_A, ACCEL_YOUT_H) / n;
    getByte(MPU_A, ACCEL_ZOUT_H);
    acc[2] += getTwoBytes(MPU_A, ACCEL_ZOUT_H) / n;
  }

  // Below code finds initial roll and pitch, which causes sideways and forward acceleration (respectively) to affect z-position
  rov.rPos[0] = acos(acc[2] / sqrt(acc[0] * acc[0] + acc[2] * acc[2]));
  rov.rPos[1] = acos(acc[2] / sqrt(acc[1] * acc[1] + acc[2] * acc[2]));
  gravity = sqrt(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]); // Save magnitude of G in LSB

  initializePressureSensor();
  initializeTemperatureSensor();
  initializeSDCard();
  initializeMotors();
}

void loop() {
  if (running) {
    move(LEFT, CLOCKWISE, 127);
    move(RIGHT, COUNTER_CLOCKWISE, 127);
    move(LEFT, CLOCKWISE, 127);
    move(RIGHT, COUNTER_CLOCKWISE, 127);
    delay(6000);
    move(LEFT, COUNTER_CLOCKWISE, 127);
    move(RIGHT, COUNTER_CLOCKWISE, 127);
    delay(500);
    move(LEFT, CLOCKWISE, 127);
    move(RIGHT, COUNTER_CLOCKWISE, 127);
    delay(5000);
    move(LEFT, CLOCKWISE, 127);
    move(RIGHT, CLOCKWISE, 127);
    delay(500);
    move(LEFT, CLOCKWISE, 127);
    move(RIGHT, COUNTER_CLOCKWISE, 127);
    delay(6000);
    stop();
    delay(30000);
  }
  else {
    if (getTwoBytes(MPU_A, ACCEL_XOUT_H) > 11000)
      running = !running;
  }
  delay(1000);
}
