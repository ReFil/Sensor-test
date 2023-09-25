#include <Arduino.h>
#include <SD.h>
#include <TimeLib.h>
#include <Wire.h>
#include "FastLED.h"

#define FIDAS
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

#define NUM_LEDS 6
#define DATA_PIN 2

CRGB leds[NUM_LEDS];

bool blink;

time_t getTeensy3Time() { return Teensy3Clock.get(); }

void setup() {

    // Init SD card for datalogging, as well as RTC, then use current date and time to initialise a
    // file to save results to
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    Serial.begin(57600);
    if (SD.begin(BUILTIN_SDCARD)) {
        Serial.println("SD card is present & ready");
    } else {
        Serial.println("SD card missing or failure");
        while (1) {
            leds[0] = CRGB::Red;
            leds[1] = CRGB::Red;
            leds[2] = CRGB::Red;
            leds[3] = CRGB::Red;
            leds[4] = CRGB::Red;
            leds[5] = CRGB::Red;
            FastLED.show();
            delay(500);
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Black;
            leds[2] = CRGB::Black;
            leds[3] = CRGB::Black;
            leds[4] = CRGB::Black;
            leds[5] = CRGB::Black;
            FastLED.show();
            delay(500);
        }
    }

    setSyncProvider(getTeensy3Time);

    filename = String(day()) + "-" + String(month()) + "-" + String(year()) + "  -  " +
               String(hour()) + "-" + String(minute()) + ".csv";
    dataFile = SD.open(filename.c_str(), FILE_WRITE);
    if (dataFile) // it opened OK
    {
        Serial.println("Writing headers to csv");
        dataFile.println("DateTime,Fidas 200 PM2.5,SPS30 PM2.5,SEN55 PM2.5,HPMA115 PM2.5,NextPM "
                         "PM2.5,SDS011 PM2.5");
        dataFile.close();
        Serial.println("Headers written");
    } else {
        Serial.println("Error opening file");
        while (1)
            ;
    }
    // Init RGB Leds
    
    FastLED.setBrightness(10);

#ifdef FIDAS
    // initialise FIDAS 200 on serial 1
    leds[0] = CRGB::Red;
    FastLED.show();
    Serial1.begin(FIDAS200_BAUD);
    fidas200.begin(&Serial1);
    fidas200.retrieveValue(PM25_1S_AVG);
    leds[0] = CRGB::Green;
#endif
#ifdef SENSPS30
    // initialise SPS30
    leds[1] = CRGB::Red;
    FastLED.show();
    Wire.begin();
    if (sps30.begin(I2C_COMMS)) {
        Serial.println("begin sps30");
    }
    sps30.probe();
    delay(2000);
    if (!sps30.probe())
        Serial.println("Failed to communicate with SPS30");
    else
        leds[1] = CRGB::Green;
    sps30.reset();
    sps30.start();
#endif
#ifdef SEN55
    // Initialise SEN55 on i2c 2
    leds[2] = CRGB::Red;
    FastLED.show();
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
    } else
        leds[2] = CRGB::Green;
#endif
#ifdef HPMA115
    // Initialise HPMA115 On serial2
    leds[3] = CRGB::Red;
    FastLED.show();
    Serial2.begin(HPMA115_BAUD);
    hpm.begin(&Serial2);
    while (!hpm.isNewDataAvailable()) {
        Serial.println("HPM still waking up...");
        delay(1000);
    }
    leds[3] = CRGB::Green;
#endif
#ifdef NEXTPM
    // Initialise NextPM on Serial3
    leds[4] = CRGB::Red;
    FastLED.show();
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
    if (data_test.connected)
        leds[4] = CRGB::Green;
#endif
#ifdef SDS011
    // Initialise SDS011
    leds[5] = CRGB::Red;
    FastLED.show();
    sds011.begin();
    if (sds011.wakeup().isWorking())
        if(sds011.queryFirmwareVersion().isOk())
            leds[5] = CRGB::Green;
#endif

    FastLED.show();
    blink = true;
}

void loop() {
    String dateTime;

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
    while (true) {
        blink = !blink;
#ifdef FIDAS
        // Read FIDAS
        leds[0] = CRGB::Red;
        FastLED.show();
        fidas200.retrieveValue(PM25_1S_AVG);
        fidasPM25 = fidas200.getValue(PM25_1S_AVG);
        Serial.println("Fidas 200 PM2.5: " + String(fidasPM25));
        if (blink)
            leds[0] = CRGB::Cyan;
        else
            leds[0] = CRGB::Green;
#endif
#ifdef SENSPS30
        // Read SPS30

        if (sps30.GetValues(&sps30val)) {
            leds[1] = CRGB::Red;
            FastLED.show();
            delay(5000);
        }
        Serial.println("SPS30 PM2.5: " + String(sps30val.MassPM2));
        if (blink)
            leds[1] = CRGB::Cyan;
        else
            leds[1] = CRGB::Green;
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
            leds[2] = CRGB::Red;
            FastLED.show();
            delay(5000);
        }
        Serial.println("SEN55 PM2.5: " + String(sen55massConcentrationPm2p5));
        Serial.println("SEN55 Teemp: " + String(sen55ambientTemperature));
        Serial.println("SEN55 RH: " + String(sen55ambientHumidity));
        if (blink)
            leds[2] = CRGB::Cyan;
        else
            leds[2] = CRGB::Green;
#endif
#ifdef HPMA115
        // Read HPMA115
        if (hpm.isNewDataAvailable()) {
            hpma115pm25 = hpm.getPM25();
        }
        Serial.println("HPMA115 PM2.5: " + String(hpma115pm25));
        if (blink)
            leds[3] = CRGB::Cyan;
        else
            leds[3] = CRGB::Green;
#endif
#ifdef NEXTPM
        // Read NextPM
        leds[4] = CRGB::Red;
        FastLED.show();
        teranextpm.fetchDataPM(nextpmdata_pm, 10);
        // teranextpm.fetchDataTH(nextpmdata_th);
        Serial.println("NEXTPM PM2.5: " + String(nextpmdata_pm.PM2_5));
        if (blink)
            leds[4] = CRGB::Cyan;
        else
            leds[4] = CRGB::Green;
#endif
#ifdef SDS011
        // Read SDS011
        leds[5] = CRGB::Red;
        FastLED.show();
        sdspm = sds011.queryPm();
        Serial.println("SDS011 PM2.5: " + String(sdspm.pm25));
        if (blink)
            leds[5] = CRGB::Cyan;
        else
            leds[5] = CRGB::Green;
#endif
        // debug printing
        FastLED.show();
        // Save to file
        dateTime = String(year()) + "-" + String(month()) + "-" + String(day()) + " " +
                   String(hour()) + ":" + String(minute()) + ":" + String(second());
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
        } else {
            Serial.println("Error opening file");
            while (1)
                ;
        }

        delay(5000);
    }
}
