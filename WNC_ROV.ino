/*/
 * Code base for the WNC CoP Underwater RoV.
 * 
 * Original Author: T. W. Herring
 *   Version 0.1.0 (November 2023)
 *     Code to sense pressure and temperature, calculate depth, read and write
 *     to onboard SD card, and drive two 12V motors via motor controller.
 * 
 * Current Author: Damian Contreras
 *   Version 0.1.1 (09 February 2023)
 *     Code base Reformatted: Pin number constants are #define'd and no longer in
 *     memory, pressure calculations recalibrated based on sensor documentation,
 *     maximum file count increased from 99 to 9999, move() reworked/shortened.
/*/

#include "RoV_Pressure.h" // In-house. Separates pressure sensor code
#include "RoV_Temperature.h" // In-house. Separates temperature probe code
#include "RoV_SDCard.h" // In-house. Separates SD card chip code
#include "RoV_Motors.h" // In-house. Separates motor code

void setup() {
  Serial.begin(9600);
  initializePressure();
  initializeTemperature();
  initializeSDCard();
  initializeMotors();
}

void loop() {
  write(getDepth() + "," + getTemperature());
  endln();
}
