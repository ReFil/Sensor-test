#include <Arduino.h>
#include <SD.h>
#include <TimeLib.h>
#include <Wire.h>
//#define FIDAS
#define SENSPS30
#define SEN55
#define HPMA115
#define NEXTPM
#define SDS011

#ifdef FIDAS
#include "FIDAS200.h"
#endif
#ifdef SENSPS30
#include "sps30.h"
#endif
#ifdef SEN55
#include "SensirionI2CSen5x.h"
#endif
#ifdef HPMA115
#include "HPMA115_Compact.h"
#endif
#ifdef NEXTPM
#include "nextpm.h"
#endif
#ifdef SDS011
#include "SdsDustSensor.h"
#endif

#ifdef FIDAS
FIDAS200 fidas200;
#endif
#ifdef SENSPS30
SPS30 sps30;
#endif
#ifdef SEN55
SensirionI2CSen5x sen5x;
#endif
#ifdef HPMA115
HPMA115_Compact hpm;
#endif
#ifdef NEXTPM
NextPM teranextpm;
#endif
#ifdef SDS011
SdsDustSensor sds011(Serial5);
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
    dataFile.println("DateTime,Fidas 200 PM2.5,SPS30 PM2.5,SEN55 PM2.5,HPMA115 PM2.5,NextPM PM2.5,SDS011 PM2.5");
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
  #ifdef SENSPS30
    //initialise SPS30
    Wire.begin();
    if(sps30.begin(I2C_COMMS)){
      Serial.println("begin sps30");
    }
    if(!sps30.probe())
      Serial.println("Failed to communicate with SPS30");
    sps30.reset();
    sps30.start();
  #endif
  #ifdef SEN55
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
  #endif
  #ifdef HPMA115
    //Initialise HPMA115 On serial2
    Serial2.begin(HPMA115_BAUD);
    hpm.begin(&Serial2);
    while (!hpm.isNewDataAvailable()) {
      Serial.println("HPM still waking up...");
      delay(1000);
    }
  #endif
  #ifdef NEXTPM
    //Initialise NextPM on Serial3
    teranextpm.begin(&Serial3);
    NextPM_test data_test;
    teranextpm.get_state();
    teranextpm.powerOnTest(data_test);
    Serial.println(data_test.connected);
	Serial.println(data_test.sleep);
	Serial.println(data_test.degraded);
	Serial.println(data_test.default_state);
	Serial.println(data_test.notready);
	Serial.println(data_test.heat_error);
	Serial.println(data_test.TH_error);
	Serial.println(data_test.fan_error);
	Serial.println(data_test.memory_error);
	Serial.println(data_test.laser_error);
  #endif
  #ifdef SDS011
    //Initialise SDS011
    sds011.begin();
    sds011.wakeup();
  #endif
}

void loop() {
  String dateTime;


  //Sensor Vars
  #ifdef FIDAS
  //Fidas
    float fidasPM25;
  #endif
  #ifdef SENSPS30
  //SPS30
    struct sps_values sps30val;
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
  #ifdef HPMA115
    //HPMA115
    uint16_t hpma115pm25;
  #endif
  #ifdef NEXTPM
    //NextPM
    NextPM_dataPM nextpmdata_pm;
    //NextPM_dataTH nextpmdata_th;
  #endif
  #ifdef SDS011
    //SDS011
    PmResult sdspm = sds011.queryPm();
  #endif
  while(true) {
  #ifdef FIDAS
    //Read FIDAS
    fidas200.retrieveValue(PM25);
    fidasPM25 = fidas200.getValue(PM25);
    Serial.println("Fidas 200 PM2.5: " + String(fidasPM25));
  #endif
  #ifdef SENSPS30
  //Read SPS30
    sps30.GetValues(&sps30val); 
    Serial.println("SPS30 PM2.5: " + String(sps30val.MassPM2));
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
  #endif
  #ifdef HPMA115
    //Read HPMA115
    if (hpm.isNewDataAvailable()) {
      hpma115pm25 = hpm.getPM25();
    }
    Serial.println("HPMA115 PM2.5: " + String(hpma115pm25));
  #endif
  #ifdef NEXTPM
    //Read NextPM
    teranextpm.fetchDataPM(nextpmdata_pm, 10);
    //teranextpm.fetchDataTH(nextpmdata_th);
    Serial.println("NEXTPM PM2.5: " + String(nextpmdata_pm.PM2_5));
  #endif
  #ifdef SDS011
    //Read SDS011
    sdspm = sds011.queryPm();
    Serial.println("SDS011 PM2.5: " + String(sdspm.pm25));
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
    #ifdef SENSPS30
    dataFile.print(String(sps30val.MassPM2) + ", ");
    #else
    dataFile.print("0, ");
    #endif
    #ifdef SEN55
    dataFile.print(String(sen55massConcentrationPm2p5) + ", ");
    #else
    dataFile.print("0, ");
    #endif
    #ifdef HPMA115
    dataFile.print(String(hpma115pm25) + ", ");
    #else
    dataFile.print("0, ");
    #endif
    #ifdef NEXTPM
    dataFile.print(String(nextpmdata_pm.PM2_5) + ", ");
    #else
    dataFile.print("0, ");
    #endif
    #ifdef SDS011
    dataFile.print(String(sdspm.pm25));
    #else
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

