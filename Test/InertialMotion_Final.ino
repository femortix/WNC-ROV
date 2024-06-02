/*/
 * Interesting quirks: Reading a register lags behind by one read operation, in terms
 * of what you actually see. If you start fresh, then read ACCEL_CONFIG and print the
 * result, it reads 0. If you then read GYRO_XOUT_H and print the result, it reads
 * what ACCEL_CONFIG was. This is why I have pointless getByte() calls just before an
 * actual variable assignments.
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
#define GYRO_CONFIG 0x1B // 0bXYZFS---; X, Y, Z, are on/off self-test bits; FS is Full Scale range for [-250 * 2^FS, 250 * 2^FS] deg/s measurement
#define ACCEL_CONFIG 0x1C // 0bXYZFS---; X, Y, Z, same concept as above; FS is again [0, 3] for [-2 * 2^FS, 2 * 2^FS] G measurement
#define ACCEL_XOUT_H 0x3B // Accel readings are in LSB, conver with G/LSB
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
#define GYRO_XOUT_H 0x43 // Gyro readings are in LSB, convert with deg/s/LSB
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
    float pos[3] = {0, 0, 0}; // Position in x-, y-, and z-axes; [s_x, s_y, s_z]
    float vel[3] = {0, 0, 0}; // Velocity in x-, y-, and z-axes; [v_x, v_y, v_z]
    float acc[3] = {0, 0, 0}; // Acceleration in forward-, left-, and up-axes; [a_f, a_l, a_u]
    float rPos[3] = {0, 0, 0}; // Roll, pitch, yaw; [phi, theta, psi]
    float rVel[3] = {0, 0, 0}; // Rotation about forward-, left-, and up-axes; [omega_f, omega_l, omega_u]

    void update(float millis) {
      float temporary[3] = {0, 0, 0};
      pos[0] += vel[0] * millis / 1000 + acc[0] / 2 * millis * millis / 1000000; // s = s_0 + v * t + 1/2 * a * t^2
      pos[1] += vel[1] * millis / 1000 + acc[1] / 2 * millis * millis / 1000000; // 1000 ms = 1 s; 1000000 ms^2 = 1 s^2
      pos[2] += vel[2] * millis / 1000 + acc[2] / 2 * millis * millis / 1000000;
      vel[0] += acc[0] * millis / 1000; // v = v_0 + a * t
      vel[1] += acc[1] * millis / 1000; // 1000 ms = 1 s
      vel[2] += acc[2] * millis / 1000;

      rPos[0] += rVel[0] * millis; // omega = omega_0 + alpha * t
      rPos[1] += rVel[1] * millis;
      rPos[2] += rVel[2] * millis;

      // Below code gets acceleration, converts them to m/s^2 if above arbitrary LSB threshold
      getByte(MPU_A, ACCEL_XOUT_H);
      temporary[0] = getTwoBytes(MPU_A, ACCEL_XOUT_H);
      if (abs(temporary[0]) <= 80) {
        temporary[0] = 0;
      else {
        getByte(MPU_A, ACCEL_CONFIG);
        temporary[0] /= 16384 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      }
      getByte(MPU_A, ACCEL_YOUT_H);
      temporary[1] = getTwoBytes(MPU_A, ACCEL_YOUT_H);
      if (abs(temporary[1]) <= 80) {
        temporary[1] = 0;
      else {
        getByte(MPU_A, ACCEL_CONFIG);
        temporary[1] /= 16384 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      }
      getByte(MPU_A, ACCEL_ZOUT_H);
      temporary[2] = getTwoBytes(MPU_A, ACCEL_ZOUT_H);
      if (abs(temporary[2]) <= 80) {
        temporary[2] = 0;
      else {
        getByte(MPU_A, ACCEL_CONFIG);
        temporary[2] /= 16384 / pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 9.8;
      }

      // Below code transforms [a_f, a_l, a_u] to [a_x, a_y, a_z] via a gross rotational matrix I had a rough time researching (take linear algebra)
      acc[0] = temporary[0]*cos(rPos[2])*cos(rPos[1]) + temporary[1]*(cos(rPos[2])*sin(rPos[1])*sin(rPos[0]) - sin(rPos[2])*cos(rPos[0])) + temporary[2]*(cos(rPos[2])*sin(rPos[1])*cos(rPos[0]) + sin(rPos[2])*sin(rPos[0]));
      acc[1] = temporary[0]*sin(rPos[2])*cos(rPos[1]) + temporary[1]*(sin(rPos[2])*sin(rPos[1])*sin(rPos[0]) + cos(rPos[2])*cos(rPos[0])) + temporary[2]*(sin(rPos[2])*sin(rPos[1])*cos(rPos[0]) - cos(rPos[2])*sin(rPos[0]));
      getByte(MPU_A, ACCEL_CONFIG); // Next line subtracts acceleration due to gravity from reading, as it isn't necessarily moving
      acc[2] = -temporary[0]*sin(rPos[1]) + temporary[1]*cos(rPos[1])*sin(rPos[0]) + temporary[2]*cos(rPos[1])*cos(rPos[0]) - gravity;

      // Below code gets rotational velocity, converts them to deg/s if above arbitrary LSB threshold
      getByte(MPU_A, GYRO_XOUT_H);
      temporary[0] = getTwoBytes(MPU_A, ACCEL_XOUT_H);
      if (abs(temporary[0]) <= 5) {
        temporary[0] = 0;
      else {
        getByte(MPU_A, GYRO_CONFIG);
        temporary[0] /= 32768 / 250 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      }
      getByte(MPU_A, GYRO_YOUT_H);
      temporary[1] = getTwoBytes(MPU_A, ACCEL_YOUT_H);
      if (abs(temporary[1]) <= 5) {
        temporary[1] = 0;
      else {
        getByte(MPU_A, GYRO_CONFIG);
        temporary[1] /= 32768 / 250 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      }
      getByte(MPU_A, GYRO_ZOUT_H);
      temporary[2] = getTwoBytes(MPU_A, ACCEL_ZOUT_H);
      if (abs(temporary[2]) <= 5) {
        temporary[2] = 0;
      else {
        getByte(MPU_A, GYRO_CONFIG);
        temporary[2] /= 32768 / 250 / pow(2, getByte(MPU_A, GYRO_CONFIG) >> 3 & 0b00000011);
      }

      // Below code transforms [omega_f, omega_l, omega_u] to [omega_x, omega_y, omega_z] by the same rotational matrix
      rVel[0] = temporary[0] * cos(rPos[2]) * cos(rPos[1]) + temporary[1] * (cos(rPos[2]) * sin(rPos[1]) * sin(rPos[0]) - sin(rPos[2]) * cos(rPos[0])) + temporary[2] * (cos(rPos[2]) * sin(rPos[1]) * cos(rPos[0]) + sin(rPos[2]) * sin(rPos[0]));
      rVel[1] = temporary[0] * sin(rPos[2]) * cos(rPos[1]) + temporary[1] * (sin(rPos[2]) * sin(rPos[1]) * sin(rPos[0]) + cos(rPos[2]) * cos(rPos[0])) + temporary[2] * (sin(rPos[2]) * sin(rPos[1]) * cos(rPos[0]) - cos(rPos[2]) * sin(rPos[0]));
      rVel[2] = -temporary[0] * sin(rPos[1]) + temporary[1] * cos(rPos[1]) * sin(rPos[0]) + temporary[2] * cos(rPos[1]) * cos(rPos[0]);
    }
};

Physical rov;

void setup() {
  Serial.begin(19200); // Differentiates it from 9600 default
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

  // Below code takes n samples and gets average accelerations at wherever its initial orientation is
  uint16_t n = 1000;
  float temporary[3] = {0, 0, 0};
  for (uint16_t i = 0; i < n; i++) {
    getByte(MPU_A, ACCEL_XOUT_H);
    acc[0] += getTwoBytes(MPU_A, ACCEL_XOUT_H) / n;
    getByte(MPU_A, ACCEL_YOUT_H);
    acc[1] += getTwoBytes(MPU_A, ACCEL_YOUT_H) / n;
    getByte(MPU_A, ACCEL_ZOUT_H);
    acc[2] += getTwoBytes(MPU_A, ACCEL_ZOUT_H) / n;
  }

  // Below code finds initial roll and pitch, assuming yaw of 0; gets measured strength of gravity in m/s^2
  rov.rPos[0] = acos(acc[2] / sqrt(acc[0] * acc[0] + acc[2] * acc[2]));
  rov.rPos[1] = acos(acc[2] / sqrt(acc[1] * acc[1] + acc[2] * acc[2]));
  getByte(MPU_A, ACCEL_CONFIG);
  gravity = sqrt(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]) * pow(2, getByte(MPU_A, ACCEL_CONFIG) >> 3 & 0b00000011) / 16384 * 9.8;
}

void loop() {
  rov.test(1);
  delay(1);
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