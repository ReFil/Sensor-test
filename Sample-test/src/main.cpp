#include <Arduino.h>
#include <SD.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <Wire.h>
#include "FastLED.h"
#include <ModbusRTUMaster.h>

//#define FIDAS
//define SENSPS30
#define SEN55
//#define HPMA115
//#define NEXTPM
//#define SDS011

#define BVOLTMULT (3.66/3.13)

#include "FIDAS200.h"
#include "sps30.h"
#include "SensirionI2CSen5x.h"
#include "HPMA115_Compact.h"
#include "nextpm.h"
#include "SdsDustSensor.h"


FIDAS200 fidas200;
SPS30 sps30;
SensirionI2CSen5x sen5x;
HPMA115_Compact hpm;
NextPM teranextpm;
SdsDustSensor sds011(Serial5);

ModbusRTUMaster modbus(Serial6);

void initSensor(void);
void sampleSensor(void);
void triggerMeas(void);
void getBattery(void);


String filename;
File dataFile;
time_t RTCTime;



bool blink;

int battery;
int batteryold;

uint8_t samplerate;

time_t getTeensy3Time() { return Teensy3Clock.get(); }

void setup() {
    Serial.begin(250000);

    delay(10000);

    // Init SD card for datalogging, as well as RTC, then use current date and time to initialise a
    // file to save results to

    pinMode(3, OUTPUT);

    modbus.begin(9600);

    if (SD.begin(BUILTIN_SDCARD)) {
        Serial.println("SD card is present & ready");
    } else {
        Serial.println("SD card missing or failure");
    }
    setSyncProvider(getTeensy3Time);

    filename = String(day()) + "-" + String(month()) + "-" + String(year()) + "  -  " +
               String(hour()) + "-" + String(minute()) + ".csv";
    dataFile = SD.open(filename.c_str(), FILE_WRITE);
    if (dataFile) // it opened OK
    {
        Serial.println("Writing headers to csv");
        dataFile.println("DateTime,PM2.5,irradiance,Battery,oldbattery,samplerate");
        dataFile.close();
        Serial.println("Headers written");
    } else {
        Serial.println("Error opening file");
        while (1);
    }
    //First sample this will get bumped up to 30
    Serial.println(filename);
    samplerate = 7;
    Serial.println("initialising");

}

void loop() {
    triggerMeas();
    Alarm.delay((3600000/samplerate));
}


void initSensor(void) {

#ifdef FIDAS
    // initialise FIDAS 200 on serial 1
    Serial1.begin(FIDAS200_BAUD);
    fidas200.begin(&Serial1);
    fidas200.retrieveValue(PM25_1S_AVG);
#endif
#ifdef SENSPS30
    // initialise SPS30
    Wire.begin();
    if (sps30.begin(I2C_COMMS)) {
        Serial.println("begin sps30");
    }
    sps30.probe();
    Alarm.delay(2000);
    if (!sps30.probe())
        Serial.println("Failed to communicate with SPS30");
    sps30.reset();
    sps30.start();
#endif
#ifdef SEN55
    // Initialise SEN55 on i2c 2
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
    // Initialise HPMA115 On serial2
    Serial2.begin(HPMA115_BAUD);
    hpm.begin(&Serial2);
    while (!hpm.isNewDataAvailable()) {
        Serial.println("HPM still waking up...");
        Alarm.delay(1000);
    }
#endif
#ifdef NEXTPM
    // Initialise NextPM on Serial3
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
    // Initialise SDS011
    sds011.begin();
    if (sds011.wakeup().isWorking())
        if(sds011.queryFirmwareVersion().isOk())
            Serial.println("SDS011 Inited");
#endif
}

void sampleSensor(void) {
    Serial.println("Sampling sensor");
    Serial.println(Alarm.getTriggeredAlarmId());
    Alarm.free(Alarm.getTriggeredAlarmId());
// Sensor Vars
#ifdef FIDAS
    // Fidas
    float fidasPM25;
#endif
#ifdef SENSPS30
    // SPS30
    struct sps_values sps30val;
#endif
#ifdef SEN55
    // SEN55
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
    // HPMA115
    uint16_t hpma115pm25;
#endif
#ifdef NEXTPM
    // NextPM
    NextPM_dataPM nextpmdata_pm;
    // NextPM_dataTH nextpmdata_th;
#endif
#ifdef SDS011
    // SDS011
    PmResult sdspm = sds011.queryPm();
#endif

#ifdef FIDAS

        fidas200.retrieveValue(PM25_1S_AVG);
        fidasPM25 = fidas200.getValue(PM25_1S_AVG);
        Serial.println("Fidas 200 PM2.5: " + String(fidasPM25));

#endif
#ifdef SENSPS30
        // Read SPS30

        if (sps30.GetValues(&sps30val)) {
            Alarm.delay(500);
        }
        Serial.println("SPS30 PM2.5: " + String(sps30val.MassPM2));
#endif
#ifdef SEN55
        // Read SEN55
        sen55error = sen5x.readMeasuredValues(
            sen55massConcentrationPm1p0, sen55massConcentrationPm2p5, sen55massConcentrationPm4p0,
            sen55massConcentrationPm10p0, sen55ambientHumidity, sen55ambientTemperature,
            sen55vocIndex, sen55noxIndex);

        if (sen55error) {
            Serial.print("Error trying to execute readMeasuredValues(): ");
            errorToString(sen55error, sen55errorMessage, 256);
            Serial.println(sen55errorMessage);
            Alarm.delay(500);
        }
        Serial.println("SEN55 PM2.5: " + String(sen55massConcentrationPm2p5));
        Serial.println("SEN55 Teemp: " + String(sen55ambientTemperature));
        Serial.println("SEN55 RH: " + String(sen55ambientHumidity));
#endif
#ifdef HPMA115
        // Read HPMA115
        if (hpm.isNewDataAvailable()) {
            hpma115pm25 = hpm.getPM25();
        }
        Serial.println("HPMA115 PM2.5: " + String(hpma115pm25));

#endif
#ifdef NEXTPM
        // Read NextPM
        teranextpm.fetchDataPM(nextpmdata_pm, 10);
        // teranextpm.fetchDataTH(nextpmdata_th);
        Serial.println("NEXTPM PM2.5: " + String(nextpmdata_pm.PM2_5));

#endif
#ifdef SDS011
        // Read SDS011
        sdspm = sds011.queryPm();
        Serial.println("SDS011 PM2.5: " + String(sdspm.pm25));

#endif

    uint16_t irradiance;
    modbus.readHoldingRegisters(1, 0, &irradiance, 1);

    Serial.println("Irradiance from reference cell" + String(irradiance));

    String dateTime;


dateTime = String(year()) + "-" + String(month()) + "-" + String(day()) + " " +
                   String(hour()) + ":" + String(minute()) + ":" + String(second());
        dataFile = SD.open(filename.c_str(), FILE_WRITE);
        if (dataFile) // it opened OK
        {
            Serial.println("Writing data to csv");
            dataFile.print(String(dateTime + ", "));
#ifdef SENSPS30
            dataFile.print(String(sps30val.MassPM2) + ", ");
#endif
#ifdef SEN55
            dataFile.print(String(sen55massConcentrationPm2p5) + ", ");
#endif
#ifdef HPMA115
            dataFile.print(String(hpma115pm25) + ", ");
#endif
#ifdef NEXTPM
            dataFile.print(String(nextpmdata_pm.PM2_5) + ", ");
#endif
#ifdef SDS011
            dataFile.print(String(sdspm.pm25) + ", ");
#endif
dataFile.print(String(irradiance) + ", ");
            dataFile.print(String(battery) + ", ");
            dataFile.print(String(batteryold) + ", ");
            dataFile.print(String(samplerate));
            dataFile.println();
            dataFile.close();
            Serial.println("data written");
        } else {
            Serial.println("Error opening file");
            while (1)
                ;
        }

Alarm.delay(500);
// shut the sensor off
digitalWrite(3, LOW);
//Alarm.alarmOnce((3600/samplerate), triggerMeas);
}

void triggerMeas(void) {
    // Take battery measurement pre sensor init, we want this to always be going up
    Serial.println("MEasurement Triggered");
    getBattery();
    Alarm.delay(250);
    //Turn sensor on
    digitalWrite(3, HIGH);
    Alarm.delay(250);
    initSensor();
    //Wait a minute for sensor to be ready
    Serial.println(Alarm.timerOnce(60, sampleSensor));
}
uint8_t battsamples = 0;

void getBattery(void) {
    battsamples++;
    Serial.print("New battery val: ");
    Serial.print(analogRead(A9)*BVOLTMULT*3.3/512);
    if(battsamples == 3) {
        battsamples = 0;
    batteryold = battery;
    battery = analogRead(A9);
    Serial.print("New battery val: ");
    Serial.print(battery*BVOLTMULT*3.3/512);
    Serial.print(battery);
    Serial.print(" Old battery: ");
    Serial.print(batteryold*BVOLTMULT*3.3/512);
    Serial.print(batteryold);
    Serial.println(" Sample rate: " + String((int)samplerate));
    if((battery - batteryold) > 2) { 
        //We've got lots of excess charge
        samplerate++;
        Serial.print("Battery voltage higher than expected, increasing sample rate by 1 SPH: ");
        Serial.print(samplerate);
        Serial.println("");
    }
    else if ((battery - batteryold) < -2){
        samplerate--;
        Serial.print("Battery voltage lower than expected, decreasing sample rate by 1 SPH: ");
        Serial.print(samplerate);
        Serial.println("");
    }
    }
}