#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>

bool bme280Init(Adafruit_BME280* bme280,uint8_t addr);
bool bh1750Init(BH1750* lightmeter, int sda,int scl);
void printAddress(DeviceAddress deviceAddress);
int ds18b20Init(DallasTemperature* probes);
float ds18b20Read(DeviceAddress deviceAddress,DallasTemperature* probes);
