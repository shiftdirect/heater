#include "Arduino.h"
#include "NVStorage.h"


CHeaterStorage::CHeaterStorage()
{
  calValues.Pmin = 14;
  calValues.Pmax = 40;
  calValues.Fmin = 1500;
  calValues.Fmax = 4500;
  calValues.ThermostatMode = 1;
  calValues.setTemperature = 22;
}

unsigned char
CHeaterStorage::getPmin()
{
  return calValues.Pmin;
}

unsigned char
CHeaterStorage::getPmax()
{
  return calValues.Pmax;
}

unsigned short
CHeaterStorage::getFmin()
{
  return calValues.Fmin;
}

unsigned short
CHeaterStorage::getFmax()
{
  return calValues.Fmax;
}

unsigned char
CHeaterStorage::getTemperature()
{
  return calValues.setTemperature;
}

unsigned char
CHeaterStorage::getThermostatMode()
{
  return calValues.ThermostatMode;
}

void
CHeaterStorage::setPmin(unsigned char val)
{
  calValues.Pmin = val;
}

void
CHeaterStorage::setPmax(unsigned char val)
{
  calValues.Pmax = val;
}

void
CHeaterStorage::setFmin(unsigned short val)
{
  calValues.Fmin = val;
}

void
CHeaterStorage::setFmax(unsigned short val)
{
  calValues.Fmax = val;
}

void
CHeaterStorage::setTemperature(unsigned char val)
{
  calValues.setTemperature = val;
}

void
CHeaterStorage::setThermostatMode(unsigned char val)
{
  calValues.ThermostatMode = val;
}


///////////////////////////////////////////////////////////////////////////////////////
//          ESP32
//
#ifdef ESP32

CESP32HeaterStorage::CESP32HeaterStorage()
{
}

CESP32HeaterStorage::~CESP32HeaterStorage()
{
  preferences.end();
}

void
CESP32HeaterStorage::init()
{
  preferences.begin("dieselheater", false);
}

void CESP32HeaterStorage::load()
{
  preferences.getBytes("calValues", &calValues, sizeof(sNVStore));
}

void CESP32HeaterStorage::save()
{
  preferences.putBytes("calValues", &calValues, sizeof(sNVStore));
}

#endif  // ESP32