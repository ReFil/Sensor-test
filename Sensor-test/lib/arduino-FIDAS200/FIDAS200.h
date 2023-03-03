#ifndef FIDAS200_H_
#define FIDAS200_H_

#include <Stream.h>
#include <stdint.h>

#define FIDAS200_BAUD 57600

// Enum for the address of all the values that can be retrieved from the FIDAS 200

enum fidas_values
{
    STATUS_BIT_SENSOR_FLOW = 0,
    STATUS_BIT_COINCIDENCE,
    STATUS_BIT_SUCTION_PUMPS,
    STATUS_BIT_WEATHER_STATION,
    STATUS_BIT_RAW_CHANNEL_DEVIATION,
    STATUS_BIT_LED_TEMP,
    STATUS_BIT_OPERATING_MODUS,
    FLOW_RATE,
    TEMPERATURE,
    HUMIDITY,
    DIFFERENTIAL_PRESSURE,
    ABSOLUTE_PRESSURE,
    FIBERTTEST,
    SUCTION_PUMP_OUTPUT,
    DIGITAL_INPUTS,
    ANALOG_IN1,
    ANALOG_IN2,
    VELOCITY,
    LED_TEMP,
    VELOCITY_FIDAS,
    COINCIDENCE_FIDAS,
    MODUS_FIDAS,
    SUCTION_PUMP_OUTPUT_FIDAS,
    IADS_TEMP_FIDAS,
    RAW_CHANNEL_DEVIATION_FIDAS,
    LED_TEMP_FIDAS,
    FLOW_RATE_FIDAS,
    WEATHERSTATION_TEMP = 40,
    WEATHERSTATION_RH,
    WEATHERSTATION_WIND_SPEED,
    WEATHERSTATION_WIND_DIRECTION,
    WEATHERSTATION_PRECIP_INTENSITY,
    WEATHERSTATION_PRECIP_TYPE,
    WEATHERSTATION_TEMP_DEW_POINT,
    WEATHERSTATION_AIR_PRESSURE,
    WEATHERSTATION_WIND_SIGNAL_QUALITY,
    PM25_1S_AVG = 52,
    PM10_1S_AVG,
    PM1_10S_AVG,
    PM25_10S_AVG,
    PM10_10S_AVG,
    PMTOT_10S_AVG,
    PM25_60S_AVG,
    PM10_60S_AVG,
    CN,
    PM1,
    PM25,
    PM4,
    PM10,
    PMTOT
};

class FIDAS200
{
public:
    FIDAS200();

    // Initialise the fidas device at the given datastream (Typically allocated to a software or hardware serial)
    bool begin(Stream *stream);

    float getValue(uint8_t index);
    void retrieveValue(uint8_t index);

private:
    Stream *fidas = NULL;

    float fidas_data[66] = {0};

    uint8_t palas_checksum(String input);
};

#endif