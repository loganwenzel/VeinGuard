#include <SoftwareSerial.h>
#include <Wire.h>
#include "MAX30105.h"

MAX30105 lightSensor_01;
MAX30105 lightSensor_02;

long sensor_data_01;
long sensor_data_02;
long count = 0;

// Set up a new serial connection on digital pins 10 and 11
SoftwareSerial BTSerial(10, 11);  // RX, TX

// Helps initilize the light sensors
void initializeSensorHR(MAX30105& sensor){
  // Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x1F; // Options: 0=Off to 255=50mA
  byte sampleAverage = 8; // Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  sensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

// helps with scaling the plot
long averageIRReadingAndPopulatePlotter(MAX30105& sensor) {
  const byte avgAmount = 64;
  long baseValue = 0;
  
  for (byte x = 0; x < avgAmount; x++) {
    baseValue += sensor.getIR(); // Read the IR value from the provided sensor
  }
  baseValue /= avgAmount;

  // Pre-populate the plotter so that the Y scale is close to IR values
  for (int x = 0; x < 500; x++) {
    Serial.println(baseValue);
  }

  return baseValue;
}

void setup() {
  // Begin the software serial communication 
  Serial.begin(115200); // Light Sensors Connection 
  BTSerial.begin(9600); // BT communication

  // ensure sensor 1 is connected
  if (!lightSensor_01.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Sensor 1 not found");
    while(1);
  }

  // ensure sensor 2 is connected
  if (!lightSensor_02.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Sensor 2 not found");
    while(1);
  }

  initializeSensorHR(lightSensor_01);
  initializeSensorHR(lightSensor_02);

  //Take an average of IR readings at power up
  long averageValue_01 = averageIRReadingAndPopulatePlotter(lightSensor_01);  
  long averageValue_02 = averageIRReadingAndPopulatePlotter(lightSensor_02);
}

void loop() {

  // makign an array to send data
  long sensor_data[3];
  sensor_data[0] = count;
  sensor_data[1] = lightSensor_01.getIR();
  sensor_data[2] = lightSensor_02.getIR();

  //Send raw data to plotter
  Serial.print(sensor_data[1]); // Print data of the second sensor
  Serial.print(",");            
  Serial.println(sensor_data[2]);// Print data of the third sensor and move to the next line

  String dataString = String(sensor_data[0]) + "," + 
                      String(sensor_data[1]) + "," + 
                      String(sensor_data[2]);
  // Send raw data to the HM-10 module
  BTSerial.println(dataString);

  // Wait for 0.1 second before sending the next data
  delay(1000);
  count += 1;
}