#include <Arduino.h>
#include <SD.h>
#include <TimeLib.h>
#include <Wire.h>

#include "SensirionI2CSen5x.h"
#include "FIDAS200.h"
#include "HPMA115_Compact.h"
#include "nextpm.h"
#include "SdsDustSensor.h"

FIDAS200 fidas200;

SensirionI2CSen5x sen5x;
HPMA115_Compact hpm;
NextPM teranextpm;
SdsDustSensor sds011(Serial4);


File dataFile;
time_t RTCTime;


time_t getTeensy3Time() {
  return Teensy3Clock.get();
}


void setup() {

  Serial.begin(57600);
  if(SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card is present & ready");
  }
  else {
    Serial.println("SD card missing or failure");
    while(1); //halt program
  }

  setSyncProvider(getTeensy3Time);


  String filename = String(day()) + "-" + String(month()) + "-" + String(year()) + "  -  " + String(hour()) + "-" + String(minute()) + ".csv";
  dataFile = SD.open(filename.c_str(), FILE_WRITE);  
   if (dataFile) // it opened OK
    {
    Serial.println("Writing headers to csv");
    dataFile.println("DateTime,Fidas 200 PM2.5,Temperature,Altitude");
    dataFile.close(); 
    Serial.println("Headers written");
    }
  else {
    Serial.println("Error opening file");  
    while(1);
  }

  //initialise FIDAS 200 on serial 1
  Serial1.begin(FIDAS200_BAUD);
  fidas200.begin(&Serial1);

  //initialise SPS30

  //Initialise SEN55 on i2c 2
  Wire1.begin();
  sen5x.begin(Wire1);

    uint16_t error;
    char errorMessage[256];
    error = sen5x.deviceReset();
    if (error) {
        Serial.print("Error trying to execute deviceReset(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    error = sen5x.startMeasurement();
    if (error) {
        Serial.print("Error trying to execute startMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
  //Initialise HPMA115 On serial2
  Serial2.begin(HPMA115_BAUD);
  hpm.begin(&Serial2);
  while (!hpm.isNewDataAvailable()) {
    Serial.println("HPM still waking up ...");
    delay(1000);
  }
  //Initialise NextPM on Serial3
  teranextpm.begin(&Serial3);
  NextPM_test data_test;
  teranextpm.powerOnTest(data_test);
  //Initialise SDS011
  sds011.begin();
  sds011.wakeup();
}

void loop() {
  String dateTime;

  //Sensor Vars
  //Fidas
    float fidasPM25;
  //SPS30

  //SEN55
    uint16_t sen55error;
    char sen55errorMessage[256];

    float sen55massConcentrationPm1p0;
    float sen55massConcentrationPm2p5;
    float sen55massConcentrationPm4p0;
    float sen55massConcentrationPm10p0;
    float sen55ambientHumidity;
    float sen55ambientTemperature;
    float sen55vocIndex;
    float sen55noxIndex;

  //HPMA115
    uint16_t hpma115pm25;

  //NextPM

  //SDS011
  PmResult sdspm = sds011.queryPm();


  //Read FIDAS
  fidas200.retrieveValue(PM25);
  Serial.println(fidas200.getValue(PM25));

  //Read SPS30

  //Read SEN55
  sen55error = sen5x.readMeasuredValues(
        sen55massConcentrationPm1p0, sen55massConcentrationPm2p5, sen55massConcentrationPm4p0,
        sen55massConcentrationPm10p0, sen55ambientHumidity, sen55ambientTemperature, sen55vocIndex,
        sen55noxIndex);

  if (sen55error) {
      Serial.print("Error trying to execute readMeasuredValues(): ");
      errorToString(sen55error, sen55errorMessage, 256);
      Serial.println(sen55errorMessage);
  }
  //Read HPMA115
  if (hpm.isNewDataAvailable()) {
    hpma115pm25 = hpm.getPM25();
  }
  //Read NextPM
  if (!nextpm.read_10sec(nextpmdata)) {
    Serial.println("Failed to read NextPM");
  } 
  //Read SDS011



  //ebugg printing

  //Save to file
  dateTime = String(year()) + "-" + String(month()) + "-" + String(day()) + " " + String(hour()) + ":" + String(minute());



  delay(1000);
}



