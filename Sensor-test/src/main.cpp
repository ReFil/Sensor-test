#include <Arduino.h>
#include <SD.h>
#include <TimeLib.h>
#include <Wire.h>
#define FIDAS
#define SEN55
#define SDS011

#ifdef FIDAS
#include "FIDAS200.h"
#endif
#ifdef SEN55
#include "SensirionI2CSen5x.h"
#endif
#ifdef SDS011
#include "SdsDustSensor.h"
#endif

#ifdef FIDAS
FIDAS200 fidas200;
#endif
#ifdef SEN55
SensirionI2CSen5x sen5x;
#endif
#ifdef SDS011
SdsDustSensor sds0111(Serial2);
SdsDustSensor sds0112(Serial3);
SdsDustSensor sds0113(Serial4);
SdsDustSensor sds0114(Serial5);

#endif

String filename;
File dataFile;
time_t RTCTime;


time_t getTeensy3Time() {
  return Teensy3Clock.get();
}


void setup() {

  //Init SD card for datalogging, as well as RTC, then use current date and time to initialise a file to save results to

  Serial.begin(57600);
  if(SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card is present & ready");
  }
  else {
    Serial.println("SD card missing or failure");
    while(1); //halt program
  }

  setSyncProvider(getTeensy3Time);


  filename = String(day()) + "-" + String(month()) + "-" + String(year()) + "  -  " + String(hour()) + "-" + String(minute()) + ".csv";
  dataFile = SD.open(filename.c_str(), FILE_WRITE);  
   if (dataFile) // it opened OK
    {
    Serial.println("Writing headers to csv");
    dataFile.println("DateTime,Fidas 200 PM2.5,SEN55 PM2.5,SEN55 RH,SEN55 T,SDS011 1 PM2.5,SDS011 2 PM2.5,SDS011 3 PM2.5,SDS011 4 PM2.5");
    dataFile.close();
    Serial.println("Headers written");
    }
  else {
    Serial.println("Error opening file");  
    while(1);
  }
  #ifdef FIDAS
    //initialise FIDAS 200 on serial 1
    Serial1.begin(FIDAS200_BAUD);
    fidas200.begin(&Serial1);
  #endif
  #ifdef SEN55
    //Initialise SEN55 on i2c 2
    Wire.begin();
    sen5x.begin(Wire);
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
  #endif
  #ifdef SDS011
    //Initialise SDS011
    sds0111.begin();
    sds0111.wakeup();
    sds0112.begin();
    sds0112.wakeup();
    sds0113.begin();
    sds0113.wakeup();
    sds0114.begin();
    sds0114.wakeup();

  #endif
}

void loop() {
  String dateTime;


  //Sensor Vars
  #ifdef FIDAS
  //Fidas
    float fidasPM25;
  #endif
  #ifdef SEN55
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
  #endif
  #ifdef SDS011
    //SDS011
    PmResult sds1pm = sds0111.queryPm();
    PmResult sds2pm = sds0112.queryPm();
    PmResult sds3pm = sds0113.queryPm();
    PmResult sds4pm = sds0114.queryPm();
  #endif
  while(true) {
  #ifdef FIDAS
    //Read FIDAS
    fidas200.retrieveValue(PM25_1S_AVG);
    fidasPM25 = fidas200.getValue(PM25_1S_AVG);
    Serial.println("Fidas 200 PM2.5: " + String(fidasPM25));
  #endif

  #ifdef SEN55
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
    Serial.println("SEN55 PM2.5: " + String(sen55massConcentrationPm2p5));
    Serial.println("SEN55 Teemp: " + String(sen55ambientTemperature));
    Serial.println("SEN55 RH: " + String(sen55ambientHumidity));
  #endif
  #ifdef SDS011
    //Read SDS011
    sds1pm = sds0111.queryPm();
    sds2pm = sds0112.queryPm();
    sds3pm = sds0113.queryPm();
    sds4pm = sds0114.queryPm();
    Serial.println("SDS011 1 PM2.5: " + String(sds1pm.pm25));
    Serial.println("SDS011 2 PM2.5: " + String(sds2pm.pm25));
    Serial.println("SDS011 3 PM2.5: " + String(sds3pm.pm25));
    Serial.println("SDS011 4 PM2.5: " + String(sds4pm.pm25));
  #endif
  //debug printing
  
  //Save to file
  dateTime = String(year()) + "-" + String(month()) + "-" + String(day()) + " " + String(hour()) + ":" + String(minute())+ ":" + String(second());
  dataFile = SD.open(filename.c_str(), FILE_WRITE);  
  if (dataFile) // it opened OK
    {
    Serial.println("Writing data to csv");
    dataFile.print(String(dateTime + ", "));
    #ifdef FIDAS
    dataFile.print(String(fidasPM25) + ", ");
    #else
    dataFile.print("0, ");
    #endif
    #ifdef SEN55
    dataFile.print(String(sen55massConcentrationPm2p5) + ", ");
    dataFile.print(String(sen55ambientHumidity) + ", ");
    dataFile.print(String(sen55ambientTemperature) + ", ");
    #else
    dataFile.print("0, ");
    dataFile.print("0, ");
    dataFile.print("0, ");
    #endif

    #ifdef SDS011
    dataFile.print(String(sds1pm.pm25) + ", ");
    dataFile.print(String(sds2pm.pm25) + ", ");
    dataFile.print(String(sds3pm.pm25) + ", ");
    dataFile.print(String(sds4pm.pm25));
    #else
    dataFile.print("0, ");
    dataFile.print("0, ");
    dataFile.print("0, ");
    dataFile.print("0");
    #endif
    dataFile.println();
    dataFile.close();
    Serial.println("data written");
    }
  else {
    Serial.println("Error opening file");  
    while(1);
  }

  delay(5000);
  }
}

