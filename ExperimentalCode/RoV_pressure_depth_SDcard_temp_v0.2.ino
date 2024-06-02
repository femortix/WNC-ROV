//Setup SD Card
#include <SD.h>
// SD uses pin 10 for chipselect
File datalog;
char filename[] = "LOGGER00.csv";
// End SD Card Setup

// Initialize Pressure sensor
int PP = 0; // pressure pin number analog pin 0
float surfaceP = 0; // variable to hold surface pressure
//End pressure sensor intialization

// Setup for digital temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 9 //This is the pin for the dallas digital temperature sensors
// Initalize Digital temperature sensor
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
// Arrays to hold device address
DeviceAddress T1;
// This ends setup for digital temperature sensor

void setup() {
  //Start Serial connection to PC
  Serial.begin(9600);

  //Start pressure sensor and intitialize surface pressure
  pinMode(PP,INPUT);
  surfaceP = 250*(float(analogRead(PP))/1023)+25;
  Serial.print("initiall surface pressure = ");
  Serial.println(surfaceP);
  // End pressure sensor initialization of surface pressure

  // Start sensors Onewire interface for Dallas Temperature Sensors
  sensors.begin();
  // Get address of digital temp sensor
  if (!sensors.getAddress(T1, 0))
  {
    Serial.println("Temp Sensor-Unable to find address for Device 0");
    }

  // these lines set the resolution of the temp sensors
  // 9 = 0.5 C, 10 = 0.25 C, 11 = 0.125, 12 = 0.0625
  sensors.setResolution(T1, 11);
  //end temperature sensor initialization

  //Start SD Card Initilization

  Serial.print("Intializing SD Card...");
  pinMode(10, OUTPUT); // Necessary for microSD breakout

  if (!SD.begin(10))
  {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("Card initialized.");

  Serial.print("Creating File...");


  // Make a new file each time the arduino is powered

  for (uint8_t i = 0; i < 100; i++)
  {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename))
    {
      // only open a new file if it doesn't exist
      datalog = SD.open(filename, FILE_WRITE);
      break;
    }
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  if (!datalog)
  {
    Serial.println("Couldn't Create File");
    return;
  }

  // Print Header - add new data columns in comma separated list in quotes of the Header string variable

  String Header = "P1(kPa),Depth(m), Temperature(C)";

  datalog = SD.open(filename, FILE_WRITE);
  datalog.println(Header);
  datalog.close();
  Serial.println("SD Card Done!");
  //end SD card initialization
}

void loop() {
  // open file on SD card
  datalog = SD.open(filename, FILE_WRITE);

  dopressuredepth();
  dotemperature();
  delay(1000);

  //close out datalog to finish a line of data
  datalog.println();
  datalog.close();

}


void dopressuredepth(){
  int Pvalue = 0;
  float Ppercent = 0;
  float Pkpa = 0;
  float depth = 0;

  Pvalue = analogRead(PP);
  Serial.println(Pvalue);
  Ppercent = float(Pvalue)/1023;
  Serial.println(Ppercent);
  Pkpa = 250*Ppercent+25;
  Serial.println(Pkpa);
  datalog.print(Pkpa);
  datalog.print(",");
  depth = (Pkpa - surfaceP)/9.81;
  Serial.print("depth = ");
  Serial.println(depth);
  datalog.print(depth);
  datalog.print(",");
}

void dotemperature(){
  sensors.requestTemperatures();
  float temp1 = sensors.getTempC(T1);
  Serial.print("T1 = ");
  Serial.print(temp1);
  Serial.println(" C");
  datalog.print(temp1);
  //datalog.print(","); //uncomment if more data columns are added after temperature
}