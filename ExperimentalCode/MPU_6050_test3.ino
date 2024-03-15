const bool GYRO = false; // Logging acceleration if false

#include <Wire.h>
#include <SD.h>

#define CHIP_SELECT 10
#define DIGITAL_INPUT 11
#define DIGITAL_OUTPUT 12
#define SPI_CLOCK 13
// Serial DAta and Serial CLock pins respectively
#define SDA 18 // I2C SDA pin, also 4 or A4
#define SCL 19 // I2C SCL pin, also 5 or A5

// Two devices can use same serial pins with different voltages. If only one device, defaults to MPU_A. Max tolerance of 6V.
#define MPU_A 0x68 // AD0 Logic Low: (1.8 +/- 5%) V
#define MPU_B 0x69 // AD0 Logic High: [2.375, 3.46] V

// MPU-6050 registers, relatively self-explanatory
#define XG_OFFSET_H 0x13 // _H registers are the "high" bytes of 2-byte pieces of data
#define XG_OFFSET_L 0x14 // _L registers are "low" bytes; combined you get 0bhhhhhhhhllllllll
#define YG_OFFSET_H 0x15 // Gyro offsets are +/- 1 LSB/deg/s
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
#define XA_OFFSET_H 0x77 // Accel offsets are +/- 0.98 gs of acceleration (???)
#define XA_OFFSET_L 0x78
#define YA_OFFSET_H 0x7A
#define YA_OFFSET_L 0x7B
#define ZA_OFFSET_H 0x7D
#define ZA_OFFSET_L 0x7E

File datalog;
char fileName[13] = "LOG_0000.csv";
int8_t getByte(int device, int address);
int16_t getTwoBytes(int device, int address);
void setByte(int device, int address, int8_t value);

void setup() {
  pinMode(CHIP_SELECT, OUTPUT);

  Serial.begin(19200);
  Serial.println("Card initializing... ");
  if (!SD.begin(CHIP_SELECT)) { // Early return: If chip can't initialize via SD.h's library
    Serial.print("Failed. Stopping.");
    return;
  }
  Serial.print("Initialized. ");

  Serial.println("Creating file... ");
  for (uint16_t i = 0; i < 10000; i++) {
    // Add each digit in a 4-digit number to '0' character code, so that 0's character code + anything from 0-9 gives '0'-'9'
    fileName[4] = i / 1000 + '0'; // ints and chars always truncate/round down fractional numbers
    fileName[5] = (i % 1000) / 100 + '0';
    fileName[6] = (i % 100) / 10 + '0';
    fileName[7] = i % 10 + '0';
    if (!SD.exists(fileName)) {
      datalog = SD.open(fileName, FILE_WRITE);
      break;
    }
  }
  if (!datalog) { // Early return: LOG_0000.csv through LOG_9999.csv exist, or datalog just couldn't be defined
    Serial.print("Failed. Stopping.");
    return;
  }
  Serial.print("Logging to: ");
  Serial.println(fileName);

  if (GYRO)
    datalog.println("xRVel (LSB),yRVel (LSB),zRVel (LSB)");
  else
    datalog.println("xAccel (LSB),yAccel (LSB),zAccel (LSB)");
  datalog.close();

  Wire.setClock(400000); // MPU 6050 communicates via 400 kHz I2C clock
  Wire.begin();
  setByte(MPU_A, PWR_MGMT_1, 0b00000001);
  setByte(MPU_A, GYRO_CONFIG, 0b00010000); // +/- 500 deg/s
  setByte(MPU_A, ACCEL_CONFIG, 0b00000000); // +/- 2 gs (+/- 19.6 m/s^2)
  setByte(MPU_A, XG_OFFSET_H, 0b00000000);
  setByte(MPU_A, XG_OFFSET_L, 0b00000000);
  setByte(MPU_A, YG_OFFSET_H, 0b00000000);
  setByte(MPU_A, YG_OFFSET_L, 0b00000000);
  setByte(MPU_A, ZG_OFFSET_H, 0b00000000);
  setByte(MPU_A, ZG_OFFSET_L, 0b00000000);
  setByte(MPU_A, XA_OFFSET_H, 0b00000000);
  setByte(MPU_A, XA_OFFSET_L, 0b00000000);
  setByte(MPU_A, YA_OFFSET_H, 0b00000000);
  setByte(MPU_A, YA_OFFSET_L, 0b00000000);
  setByte(MPU_A, ZA_OFFSET_H, 0b00000000);
  setByte(MPU_A, ZA_OFFSET_L, 0b00000000);
}

void loop() {
  datalog = SD.open(fileName, FILE_WRITE);
  if (GYRO) {
    getByte(MPU_A, GYRO_XOUT_H);
    datalog.print(getTwoBytes(MPU_A, GYRO_XOUT_H));
    datalog.print(",");
    getByte(MPU_A, GYRO_YOUT_H);
    datalog.print(getTwoBytes(MPU_A, GYRO_YOUT_H));
    datalog.print(",");
    getByte(MPU_A, GYRO_ZOUT_H);
    datalog.print(getTwoBytes(MPU_A, GYRO_ZOUT_H));
  }
  else {
    getByte(MPU_A, ACCEL_XOUT_H);
    datalog.print(getTwoBytes(MPU_A, ACCEL_XOUT_H));
    datalog.print(",");
    getByte(MPU_A, ACCEL_YOUT_H);
    datalog.print(getTwoBytes(MPU_A, ACCEL_YOUT_H));
    datalog.print(",");
    getByte(MPU_A, ACCEL_ZOUT_H);
    datalog.print(getTwoBytes(MPU_A, ACCEL_ZOUT_H));
  }
  datalog.println();
  datalog.close();
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
