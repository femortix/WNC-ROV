/*/
 * NOTES ON (HOPEFULLY) FINAL REVISION:
 *   The acceleration offsets did nothing when I tweaked them. They were weird to begin with, being "offset
 * cancellation" measured in "9.8E-4 Gs of acceleration" per step. Then I realized I was reading registers for
 * the MPU-6500 (the gyro offsets still worked as expected).
 *   After serial outputting the offset values for the gyroscope and manually-offset accelerometer, I give the
 * values my seal of approval. Unsure if the electrical noise will average to 0 as it continues running, or
 * if I should convert them to human-readable units then round them off at ~0.08 deg/s (5 LSB) and ~0.06 m/s^2
 *(100 LSB) respectively.
/*/

#include <Wire.h>

// Serial DAta and Serial CLock pins respectively
#define SDA 18 // I2C SDA pin, also 4 or A4
#define SCL 19 // I2C SCL pin, also 5 or A5

// Two devices can use same serial pins with different voltages. If only one device, defaults to MPU_A. Max tolerance of 6V.
#define MPU_A 0x68 // AD0 Logic Low: (1.8 +/- 5%) V
#define MPU_B 0x69 // AD0 Logic High: [2.375, 3.46] V

// MPU-6050 registers, relatively self-explanatory
#define XG_OFFSET_H 0x13 // _H registers are the "high" bytes of 2-byte pieces of data
#define XG_OFFSET_L 0x14 // _L registers are "low" bytes; combined you get 0bhhhhhhhhllllllll
#define YG_OFFSET_H 0x15 // Gyro offsets are +/- 1 LSB
#define YG_OFFSET_L 0x16
#define ZG_OFFSET_H 0x17
#define ZG_OFFSET_L 0x18
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48
#define PWR_MGMT_1 0x6B

int8_t getByte(int device, int address);
int16_t getTwoBytes(int device, int address);
void setByte(int device, int address, int8_t value);

void setup() {
  Serial.begin(19200);
  Wire.setClock(400000); // MPU 6050 communicates via 400 kHz I2C clock
  Wire.begin();
  setByte(MPU_A, PWR_MGMT_1, 0b00000001); // Reset? Really don't know what this does
  setByte(MPU_A, GYRO_CONFIG, 0b00010000); // [-500, 500] deg/s over [-32768, 32767]
  setByte(MPU_A, ACCEL_CONFIG, 0b00000000); // [-2, 2] Gs over [-32768, 32767] 
  setByte(MPU_A, XG_OFFSET_H, 0b00000000); // Add 92 bits to GYRO_XOUT
  setByte(MPU_A, XG_OFFSET_L, 0b01011100);
  setByte(MPU_A, YG_OFFSET_H, 0b00000000); // Add 33 bits to GYRO_YOUT
  setByte(MPU_A, YG_OFFSET_L, 0b00100001);
  setByte(MPU_A, ZG_OFFSET_H, 0b11111111); // Subtract 7 bits from GYRO_ZOUT
  setByte(MPU_A, ZG_OFFSET_L, 0b11111001);
}

void loop() {
  // Thought I should mention I use int16_t, since the behavior of short varies (2 or 4 bytes) by device
  int16_t acc[3];
  getByte(MPU_A, ACCEL_XOUT_H);
  acc[0] = getTwoBytes(MPU_A, ACCEL_XOUT_H);
  getByte(MPU_A, ACCEL_YOUT_H);
  acc[1] = getTwoBytes(MPU_A, ACCEL_YOUT_H);
  getByte(MPU_A, ACCEL_ZOUT_H);
  acc[2] = getTwoBytes(MPU_A, ACCEL_ZOUT_H);

  Serial.print("ACC: ");
  Serial.print(acc[0] + 4);
  Serial.print(", ");
  Serial.print(acc[1] + 4);
  Serial.print(", ");
  Serial.println(acc[2] - 14720);
  delay(100);
}

int8_t getByte(int device, int address) {
  Wire.beginTransmission(device);
  Wire.write(address);
  Wire.requestFrom(device, 1, true);
  Wire.endTransmission(true);
  return Wire.read();
}

int16_t getTwoBytes(int device, int address) {
  Wire.beginTransmission(device);
  Wire.write(address);
  Wire.requestFrom(device, 2, true);
  Wire.endTransmission(true);
  return Wire.read() << 8 | Wire.read();
}

void setByte(int device, int address, int8_t value) {
  Wire.beginTransmission(device);
  Wire.write(address);
  Wire.write(value);
  Wire.endTransmission(true);
}
