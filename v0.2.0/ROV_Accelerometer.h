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

float gravity;

class Physical {
  public:
    float pos[3] = {0, 0, 0};
    float vel[3] = {0, 0, 0};
    float acc[3] = {0, 0, 0};
    float rPos[3] = {0, 0, 0};
    float rVel[3] = {0, 0, 0};

    void update(float seconds) {
      float temporary[3] = {0, 0, 0};
      pos[0] += vel[0] * seconds + acc[0] / 2 * seconds * seconds;
      pos[1] += vel[1] * seconds + acc[1] / 2 * seconds * seconds;
      pos[2] += vel[2] * seconds + acc[2] / 2 * seconds * seconds;
      vel[0] += acc[0] * seconds;
      vel[1] += acc[1] * seconds;
      vel[2] += acc[2] * seconds;

      rPos[0] += rVel[0] * seconds;
      rPos[1] += rVel[1] * seconds;
      rPos[2] += rVel[2] * seconds;

      // Below code gets acceleration, converts them to m/s^2 if above 100 LSB threshold
      getByte(MPU_A, ACCEL_XOUT_H);
      temporary[0] = getTwoBytes(MPU_A, ACCEL_XOUT_H);
      if (abs(temporary[0]) <= 100) {
        temporary[0] = 0;
      }
      else {
        getByte(MPU_A, ACCEL_CONFIG);
        temporary[0] /= 16384 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      }
      getByte(MPU_A, ACCEL_YOUT_H);
      temporary[1] = getTwoBytes(MPU_A, ACCEL_YOUT_H);
      if (abs(temporary[1]) <= 100) {
        temporary[1] = 0;
      }
      else {
        getByte(MPU_A, ACCEL_CONFIG);
        temporary[1] /= 16384 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      }
      getByte(MPU_A, ACCEL_ZOUT_H);
      temporary[2] = getTwoBytes(MPU_A, ACCEL_ZOUT_H);
      if (abs(temporary[2]) <= 100) {
        temporary[2] = 0;
      }
      else {
        getByte(MPU_A, ACCEL_CONFIG);
        temporary[2] /= 16384 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      }

      // Below code filters forward, sideways, and vertical acceleration to absolute x, y, and z acceleration
      acc[0] = temporary[0] * cos(rPos[1]) * cos(rPos[2]) + temporary[1] * sin(rPos[2]) * cos(rPos[0]) + temporary[2] * sin(rPos[1]) * cos(rPos[2]);
      acc[1] = temporary[1] * cos(rPos[0]) * cos(rPos[2]) + temporary[0] * sin(rPos[2]) * cos(rPos[1]) + temporary[2] * sin(rPos[0]) * cos(rPos[2]);
      getByte(MPU_A, ACCEL_CONFIG); // The below line of code shows how sick in the head I am, and it's too much to explain here
      acc[2] = temporary[2] * cos(rPos[0]) * cos(rPos[1]) + temporary[0] * sin(rPos[1]) * cos(rPos[0]) + temporary[1] * sin(rPos[0]) * cos(rPos[1]) - gravity * pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 16384 * 9.8;

      // Below code gets rotational velocity, converts them to deg/s if above 5 LSB threshold
      getByte(MPU_A, GYRO_XOUT_H);
      temporary[0] = getTwoBytes(MPU_A, ACCEL_XOUT_H);
      if (abs(temporary[0]) <= 5) {
        temporary[0] = 0;
      }
      else {
        getByte(MPU_A, GYRO_CONFIG);
        temporary[0] /= 32768 / 250 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      }
      getByte(MPU_A, GYRO_YOUT_H);
      temporary[1] = getTwoBytes(MPU_A, ACCEL_YOUT_H);
      if (abs(temporary[1]) <= 5) {
        temporary[1] = 0;
      }
      else {
        getByte(MPU_A, GYRO_CONFIG);
        temporary[1] /= 32768 / 250 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      }
      getByte(MPU_A, GYRO_ZOUT_H);
      temporary[2] = getTwoBytes(MPU_A, ACCEL_ZOUT_H);
      if (abs(temporary[2]) <= 5) {
        temporary[2] = 0;
      }
      else {
        getByte(MPU_A, GYRO_CONFIG);
        temporary[2] /= 32768 / 250 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      }

      // Below code filters roll, pitch, and yaw angular velocity to absolute theta, phi, and psi rotation
      rVel[0] = temporary[0] * cos(rVel[1]) * cos(rVel[2]) + temporary[1] * cos(rVel[1]) * sin(rVel[2]) + temporary[2] * cos(rVel[2]) * sin(rVel[1]);
      rVel[1] = temporary[1] * cos(rVel[0]) * cos(rVel[2]) + temporary[0] * cos(rVel[0]) * sin(rVel[2]) + temporary[2] * cos(rVel[2]) * sin(rVel[0]);
      rVel[2] = temporary[2] * cos(rVel[0]) * cos(rVel[1]) + temporary[0] * cos(rVel[0]) * sin(rVel[1]) + temporary[1] * cos(rVel[1]) * sin(rVel[0]);
    }
};

Physical rov;

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