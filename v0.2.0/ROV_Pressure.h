#define PRESSURE 14 // Analog necessary (can also be called 0 or A0). Pressure sensor input

float surface = 0;

void initializePressureSensor() {
  pinMode(PRESSURE, INPUT);

  surface = 230 * (float(analogRead(PRESSURE)) / 1023) + 20;
  Serial.print("Initial Surface Pressure: ");
  Serial.print(surface);
  Serial.println(" kPa");
}

float getPressure() {
  return 230 * (float(analogRead(PRESSURE)) / 1023) + 20; // 0-1023 analog read, to 0-1 range, scaled to 0-230, edited to 20-250. Sensor reads 20-250 kPa
}

float getDepth() {
  float depth = 0;

  depth = (getPressure() - surface) / 9.8; // kPa is kN/m^2, 1 m^3 of water gives 1000 kg * 9.8 m/s^2 more Newtons, or 9.8 more kPa per m of depth
  Serial.print("Depth: ");
  Serial.print(depth);
  Serial.println(" m");

  return(depth);
}