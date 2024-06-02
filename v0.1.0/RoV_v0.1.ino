/* 
This is code to run the WNC CoP underwater RoV.
Authors: TW Herring, 
version 0.1 (November 2023) includes:
a pressure sensor to calculate depth
a temperature sensor
an SD card reader/writer to record data (depth and temperature)
and a motor controller to drive two motors

Further improvements will be listed here
*/

//Setup SD Card
#include <SD.h>
/* SD uses pin 10 for chipselect connections should be as follows
CS - 10
DI (MOSI) - 11
DO (MISO) - 12
CLK - 13
GND - GND
VCC - 5V
*/

// The following sets up the SD card File object which is called to write data to the card
File datalog;
char filename[] = "LOGGER00.csv";
// End SD Card Setup

/*
The pressure sensor is a MPX4250AP sensor. It has 6 pins but only 3 should be connected in this application.
Looking from the back of the sensor body the port should be on the right side.
Numbering pins 1 - 6, starting at the left side, connect the pins as follows
Pin 1 - A0
Pin 2 - GND
Pin 3 - 5V
Pin 4 - Do not connect to ground or external circuitry!
Pin 5 - Do not connect to ground or external circuitry!
Pin 6 - Do not connect to ground or external circuitry!
*/
// Initialize Pressure sensor
int PP = 0; // pressure pin number analog pin 0 (aka A0)
float surfaceP = 0; // variable to hold surface pressure
float depth = 0; // variable to hold depth data
//End pressure sensor intialization

/*
The Dallas DS18B20 temperature sensor in a water proof package such as 
HiLetgo 5pcs DS18B20 Temperature Sensor Temperature Probe Stainless Steel Package Waterproof 1M (amazon.com)
should be connected as follows:
Red wire - 5V
Yellow or Orange Wire Signal - 9
Black - GND
*/

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
float temperature = 0; //variable to hold temperature data
// This ends setup for digital temperature sensor

/*
Setup TB6612FNG dual H-bridge for motor control

H-bridge should be connected to arduino pins as follows:
VCC - 5V
GND - GND
STBY - 2
PWMA - 3
AIN1 - 2
AIN2 - 4
PWMB - 6
BIN1 - 7
BIN2 - 8

A seperate motor power supply should be connected to the H-Bridge as follows:
GND - GND (also to arduino GND)
Motor High Voltage - VM

The motor leads should be connected to the H-Bridge as follows:
motor A should be connected between AO1 and AO2
motor B should be connected between BO1 and BO2
*/
//The below code defines the output pins on the Arduino will hookup to specified pins on the HBridge as detailed above
int STBY = 5; //this will be the standby pin
//Motor A
int PWMA = 3; //Speed control
int AIN1 = 2; //Direction
int AIN2 = 4; //Direction

//Motor B
int PWMB = 6; //Speed control
int BIN1 = 7; //Direction
int BIN2 = 8; //Direction
// This ends motoro controller setup

void setup() {
  //Start Serial connection to PC
  Serial.begin(9600);

  //Start pressure sensor and intitialize surface pressure
  pinMode(PP,INPUT);
  surfaceP = 250*(float(analogRead(PP))/1023)+25;
  Serial.print("initial surface pressure = ");
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

  String Header = "Depth(m), Temperature(C)";

  datalog = SD.open(filename, FILE_WRITE);
  datalog.println(Header);
  datalog.close();
  Serial.println("SD Card Done!");
  //end SD card initialization

  // Inititalize pin modes for H-bridge motor control
  pinMode(STBY, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  // End H-bridge otor control pin initialization
}

void loop() {
  // open file on SD card
  datalog = SD.open(filename, FILE_WRITE);

  // get depth and record to SD card
  depth = getdepth();
  datalog.print(depth);
  datalog.print(",");

  // get temperature and record to SD card
  temperature = gettemperature();
  datalog.print(temperature);
  // datalog.print(","); //unccomment this line if adding more data columns

  //close out datalog to finish a line of data
  datalog.println();
  datalog.close();

  // insert some move commands maybe based on depth and/or temperature values
  //If the depth is less than 2 m turn on both motors full speed
  move(1,255,1);
  move(2,255,1);
  delay(1000);
  stop();

 
 
 /* if(depth <= 2.0){
    move(1,255,1);
    move(2,255,1);
  }
  else{
    stop();
  }
  */
  
  // delay some number of milliseconds before repeating
  delay(100);

}

// Here is a function to determine depth using the pressure sensor
float getdepth(){
  float howdeep = 0;
  howdeep = (250*(float(analogRead(PP))/1023)+25 - surfaceP)/9.81;
  Serial.print("depth = ");
  Serial.println(howdeep);
  return(howdeep);
}

// Here is a function to get the temperature from the sensor
float gettemperature(){
  sensors.requestTemperatures();
  float temp1 = sensors.getTempC(T1);
  Serial.print("T1 = ");
  Serial.print(temp1);
  Serial.println(" C");
  return(temp1);
}

// Here is a function to turn on a motor 
void move(int motor, int speed, int direction){
//Move specific motor at speed and direction
//motor: 0 for B 1 for A
//speed: 0 is off, and 255 is full speed
//direction: 0 clockwise, 1 counter-clockwise
  digitalWrite(STBY, HIGH); //disable standby
  boolean inPin1 = LOW;
  boolean inPin2 = HIGH;

  if(direction == 1){
    inPin1 = HIGH;
    inPin2 = LOW;
  }

  if(motor == 1){
    digitalWrite(AIN1, inPin1);
    digitalWrite(AIN2, inPin2);
    analogWrite(PWMA, speed);
  }else{
    digitalWrite(BIN1, inPin1);
    digitalWrite(BIN2, inPin2);
    analogWrite(PWMB, speed);
  }
}

// Here is a function to stop both motors
void stop(){
//enable standby 
  digitalWrite(STBY, LOW);
}
