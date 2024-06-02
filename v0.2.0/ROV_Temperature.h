#include <OneWire.h> // Used by DallasTemperature.h
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 9 // Temperature sensor input

DeviceAddress T1; // Temperature sensor address array
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); // Just the address of the OneWire pin

void initializeTemperatureSensor() {
  sensors.begin();
  if (!sensors.getAddress(T1, 0)) { // Early return: If temperature sensor cannot be found
    Serial.println("Temperature Sensor not detected. Stopping.");
    return;
  }

  // Temperature sensor docs claim associated precision bits always in memory anyways
  sensors.setResolution(T1, 12);// int is bit resoluton for temperature data: 9 to 12 bits, down to 2^-1 (0.5 C) to 2^-4 (0.0625 C) bits respectively
}

float getTemperature() {
  float temp = sensors.getTempC(T1);

  sensors.requestTemperatures();
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");

  return temp;
}