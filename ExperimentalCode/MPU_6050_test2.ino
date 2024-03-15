/*/
 * UPDATES FROM 1ST REVISION:
 *   Removed over-engineered pieces of code. Reset PWR_MGMT_1 on chip restart. Tweaks/fixes for Physical.update()
 *   I had "temporary << 8" instead of "temporary = temporary << 8" in getTwoBytes() meaning the 8 most significant bits could
 * never be filled, and the 8 least significant bits would be bogus, but I decided to just make it all one line rather than break
 * things down overly-much.
 *   Now then, when I printed out getTwoBytes(MPU_A, GYRO_XOUT_H), I get a number at about -85 +/- 5 at standstill, but it
 * increased as it rotated counter-clockwise, and decreased as it did clockwise. When I set rVel[0] to the same function call,
 * then immediately print it out, it instead would spit out 2048.00 at all times. rVel[0] was actually being set to the two bytes
 * starting at ACCEL_CONFIG, as 2048 in binary is 00001000 00000000, and ACCEL_CONFIG is 00001000 with the following register
 * being empty. I confirmed this after setting ACCEL_CONFIG to 00010000 and seeing rVel[0] become 4096.00.
 *   After more fiddling around, I found out that ACCEL_CONFIG was the last register address referenced by the program, before
 * attempting to access GYRO_XOUT_H, and sure enough, adding vestigial getByte() calls in as buffers fixed that. How do the two
 * byte reading functions not update what register they should be reading, until the second time they're told to do so?
/*/

#include <Wire.h>

// Serial DAta and Serial CLock pins respectively
#define SDA 18 // I2C SDA pin, also 4 or A4
#define SCL 19 // I2C SCL pin, also 5 or A5

// Two devices can use same serial pins with different voltages. If only one device, defaults to MPU_A. Max tolerance of 6V.
#define MPU_A 0x68 // AD0 Logic Low: (1.8 +/- 5%) V
#define MPU_B 0x69 // AD0 Logic High: [2.375, 3.46] V

// MPU-6050 registers, relatively self-explanatory
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define ACCEL_XOUT_H 0x3B // _H registers are the "high" bytes of 2-byte pieces of data
#define ACCEL_XOUT_L 0x3C // _L registers are "low" bytes; combined you get 0bhhhhhhhhllllllll
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

class Physical {
  public:
    float pos[3] = {0, 0, 0};
    float vel[3] = {0, 0, 0};
    float acc[3] = {0, 0, 0};
    float rPos[3] = {0, 0, 0};
    float rVel[3] = {0, 0, 0};
    void update(float seconds) {
      // Apply changes from current values being applied over some amount of time seconds
      pos[0] += acc[0] / 2 * seconds * seconds + vel[0] * seconds;
      pos[1] += acc[1] / 2 * seconds * seconds + vel[1] * seconds;
      pos[2] += acc[2] / 2 * seconds * seconds + vel[2] * seconds;
      vel[0] += acc[0] * seconds;
      vel[1] += acc[1] * seconds;
      vel[2] += acc[2] * seconds;
      rPos[0] += rVel[0] * seconds;
      rPos[1] += rVel[1] * seconds;
      rPos[2] += rVel[2] * seconds;

      // Gather up-to-date raw acceleration readings
      getByte(MPU_A, ACCEL_XOUT_H);
      acc[0] = getTwoBytes(MPU_A, ACCEL_XOUT_H);
      getByte(MPU_A, ACCEL_CONFIG);
      acc[0] /= 16384.0 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      getByte(MPU_A, ACCEL_YOUT_H);
      acc[1] = getTwoBytes(MPU_A, ACCEL_YOUT_H);
      getByte(MPU_A, ACCEL_CONFIG);
      acc[1] /= 16384.0 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      getByte(MPU_A, ACCEL_ZOUT_H);
      acc[2] = getTwoBytes(MPU_A, ACCEL_ZOUT_H);
      getByte(MPU_A, ACCEL_CONFIG);
      acc[2] /= 16384.0 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;

      // Assign new up-to-date readings for rotational velocity
      getByte(MPU_A, GYRO_XOUT_H);
      rVel[0] = getTwoBytes(MPU_A, GYRO_XOUT_H);
      getByte(MPU_A, GYRO_CONFIG);
      rVel[0] /= 32768.0 / 250.0 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      getByte(MPU_A, GYRO_YOUT_H);
      rVel[1] = getTwoBytes(MPU_A, GYRO_YOUT_H);
      getByte(MPU_A, GYRO_CONFIG);
      rVel[1] /= 32768.0 / 250.0 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      getByte(MPU_A, GYRO_ZOUT_H);
      rVel[2] = getTwoBytes(MPU_A, GYRO_ZOUT_H);
      getByte(MPU_A, GYRO_CONFIG);
      rVel[2] /= 32768.0 / 250.0 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);

      // Filter raw accelerations using current rotational position, worked out the formula on a piece of paper
      //acc[0] = acc[0] * cos(rPos[1]) * cos(rPos[2]) + acc[1] * sin(rPos[2]) + acc[2] * sin(rPos[1]);
      //acc[1] = acc[1] * cos(rPos[0]) * cos(rPos[2]) + acc[0] * sin(rPos[2]) + acc[2] * sin(rPos[0]);
      //acc[2] = acc[2] * cos(rPos[0]) * cos(rPos[1]) + acc[0] * sin(rPos[1]) + acc[1] * sin(rPos[0]);
    }
};

Physical test;

void setup() {
  Serial.begin(19200);
  Wire.setClock(400000); // MPU 6050 communicates via 400 kHz I2C clock
  Wire.begin();
  setByte(MPU_A, PWR_MGMT_1, 0b00000001);
  setByte(MPU_A, GYRO_CONFIG, 0b00010000);
  setByte(MPU_A, ACCEL_CONFIG, 0b00000000);
}

void loop() {
  test.update(1);
  //Serial.print("X-omega: ");
  //Serial.print(test.rVel[0]);
  //Serial.print(", Y-omega: ");
  //Serial.print(test.rVel[1]);
  //Serial.print(", Z-omega: ");
  //Serial.println(test.rVel[2]);
  Serial.print("X-a: ");
  Serial.print(test.acc[0]);
  Serial.print(", Y-a: ");
  Serial.print(test.acc[1]);
  Serial.print(", Z-a: ");
  Serial.println(test.acc[2]);
  delay(1000);
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
