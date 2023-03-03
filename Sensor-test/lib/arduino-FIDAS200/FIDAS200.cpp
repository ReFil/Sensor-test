#include <Arduino.h>
#include <stdio.h>
#include "FIDAS200.h"

FIDAS200::FIDAS200() {
}

bool FIDAS200::begin(Stream *stream) {
  fidas = stream;

  return true;
}

uint8_t FIDAS200::palas_checksum(String input) {

}

float FIDAS200::getValue(uint8_t index) {
    return fidas_data[index];
}


void FIDAS200::retrieveValue(uint8_t index) {
    String toSend = String("<getVal " + String(index) + ">");
    //toSend += String(palas_checksum(toSend));
    fidas->write(toSend.c_str());
    while(fidas->available() < 21);
    String readIn = fidas->readStringUntil('>');
    //Serial.println(readIn);
    readIn.remove(0, readIn.lastIndexOf('=')+1);
    //Serial.println(readIn);
    if(index >= PM25_1S_AVG && index != CN)
        fidas_data[index] = readIn.toFloat()*1000;
    else
        fidas_data[index] = readIn.toFloat();


}