/*/
 * Experimental because I don't know if the code works. Reading a register on the MPU-6050 should
 * return 1 byte of data (an integer from -128 to 127). It returns -1 when the register or device
 * doesn't exist. Currently our MPU-6050 is returning 0 for every direction, and doesn't change when
 * physically moved around.
 * If you can find an error in my code that would cause this, that would be appreciated. If not, maybe
 * this file can still give newer programmers some spooky stuff to sift through or ask about. Lots of
 * comments, partly to ensure I wasn't the crazy one when stuff didn't work.
/*/

#include <Wire.h>

// Serial DAta and Serial CLock pins respectively
#define SDA 18 // I2C SDA pin, also 4 or A4
#define SCL 19 // I2C SCL pin, also 5 or A5

// Two devices can use same serial pins with different voltages. If only one device, defaults to MPU_A. Max tolerance of 6V
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

int8_t getByte(int device, int address);
int16_t getTwoBytes(int device, int address);
void setByte(int device, int address, int8_t value);

class Physical {
  public:
    float pos[3] = {0, 0, 0};
    float vel[3] = {0, 0, 0};
    float acc[3]; // This and rVel are directly assigned, not calculated from a base value
    float rPos[3] = {0, 0, 0};
    float rVel[3];
    void update(float seconds) {
      float temporary[3];

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
      temporary[0] = getTwoBytes(MPU_B, ACCEL_XOUT_H); // 16384, 8192, 4096, or 2048 LSB/g based on ACCEL_CONFIG's AFS_SEL bits (0bxyzSS---)
      temporary[0] = temporary[0] / 16384 * pow(2, (getByte(MPU_B, ACCEL_CONFIG) >> 3) & 0b00000011) * 9.8; // LSB / LSB/g * (m/s^2)/g = m/s^2
      temporary[1] = getTwoBytes(MPU_B, ACCEL_YOUT_H);
      temporary[1] = temporary[1] / 16384 * pow(2, (getByte(MPU_B, ACCEL_CONFIG) >> 3) & 0b00000011) * 9.8;
      temporary[2] = getTwoBytes(MPU_B, ACCEL_ZOUT_H);
      temporary[2] = temporary[2] / 16384 * pow(2, (getByte(MPU_B, ACCEL_CONFIG) >> 3) & 0b00000011) * 9.8;

      // Assign new up-to-date readings for rotational velocity
      rVel[0] = getTwoBytes(MPU_B, GYRO_XOUT_H); // ~131, ~65.5, ~33, ~16.5 LSB/(deg/s) based on GYRO_CONFIG's FS_SEL bits (0b---SS---)
      rVel[0] = rVel[0] / (32768 / 250 / pow(2, getByte(MPU_B, GYRO_CONFIG) >> 3)); // LSB / LSB/(deg/s) = deg/s
      rVel[1] = getTwoBytes(MPU_B, GYRO_YOUT_H);
      rVel[1] = rVel[1] / (32768 / 250 / pow(2, getByte(MPU_B, GYRO_CONFIG) >> 3));
      rVel[2] = getTwoBytes(MPU_B, GYRO_ZOUT_H);
      rVel[2] = rVel[2] / (32768 / 250 / pow(2, getByte(MPU_B, GYRO_CONFIG) >> 3));

      // Filter raw accelerations using current rotational position, worked out the formula on a piece of paper
      acc[0] = temporary[0] * cos(rPos[1]) * cos(rPos[2]) + temporary[1] * sin(rPos[2]) + temporary[2] * sin(rPos[1]);
      acc[1] = temporary[1] * cos(rPos[0]) * cos(rPos[2]) + temporary[0] * sin(rPos[2]) + temporary[2] * sin(rPos[0]);
      acc[2] = temporary[2] * cos(rPos[0]) * cos(rPos[1]) + temporary[0] * sin(rPos[1]) + temporary[1] * sin(rPos[0]);
    }
};

Physical test;

void setup() {
  Serial.begin(19200);
  Wire.setClock(400000); // MPU 6050 communicates via 400 kHz I2C clock
  Wire.begin();
  setByte(MPU_A, GYRO_CONFIG, 0b00000000);
  setByte(MPU_A, ACCEL_CONFIG, 0b00000000);
}

void loop() {
  Wire.beginTransmission(MPU_A);
  Wire.write(GYRO_XOUT_H);
  Wire.requestFrom(MPU_A, 1, true);
  Serial.println(Wire.read());
  Wire.write(GYRO_XOUT_L);
  Wire.requestFrom(MPU_A, 1, true);
  Serial.println(Wire.read());
  Wire.endTransmission(true);
  //test.update(0.01); // 10 ms period
  delay(10);
}

int8_t getByte(int device, int address) {
  Wire.beginTransmission(device);
  Wire.write(address); // First write() on an MPU brings the current address pointer to that spot
  Wire.requestFrom(device, 1, true);
  Wire.endTransmission(true);

  return Wire.read();
}

int16_t getTwoBytes(int device, int address) {
  int16_t temporary = 0;

  Wire.beginTransmission(device);
  Wire.write(address);
  Wire.requestFrom(device, 2, true);
  temporary = Wire.read(); // Set to the higher byte of data, advance the pointer to the next register
  temporary << 8; // Make room for the lower byte
  temporary |= Wire.read(); // Fill lower 8 bits with lower byte
  Wire.endTransmission(true);

  return temporary;
}

void setByte(int device, int address, int8_t value) {
  Wire.beginTransmission(device);
  Wire.write(address);
  Wire.write(value); // Second write() on an MPU actually overwrites the address at the pointer
  Wire.endTransmission(true);
}
