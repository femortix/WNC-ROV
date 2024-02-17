#include <SD.h> // Used to interface with SD card chip

#define CHIP_SELECT 10 // SBI bus, pin 10 hard-coded into SD.h (53 in Arduino Mega). Initializes SD card
#define DIGITAL_INPUT 11 // Pin 11 required for SBI bus (50 in Arduino Mega). File input
#define DIGITAL_OUTPUT 12 // Pin 12 required for SBI bus (51 in Arduino Mega). File output
#define SPI_CLOCK 13 // Pin 13 required for SBI bus (52 in Artuino Mega). Synchronizes communication between SD card and device

File datalog;
// File name hard-coded to a max of 13 characters in SD.h
char fileName[13] = "LOG_0000.csv"; // Strings in C/C++ have a hidden exit character appended at the end, so this has 13 characters

void initializeSDCard() {
  pinMode(CHIP_SELECT, OUTPUT);

  Serial.println("Card initializing... ");
  if (!SD.begin(CHIP_SELECT)) { // Early return: If chip can't initialize via SD.h's library
    Serial.print("Failed. Stopping.");
    return;
  }
  Serial.print("Initialized.");

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

  datalog.println("Depth (m), Temperature (C)");
  datalog.close();
}

void write(char text[]) {
  datalog = SD.open(fileName, FILE_WRITE);

  datalog.print(text);
  datalog.close();
}

void endln() {
  datalog.println();
}
