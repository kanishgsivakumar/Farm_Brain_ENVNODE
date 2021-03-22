
#include <OneWire.h>
#include <DallasTemperature.h>


void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

int ds18b20Init(DallasTemperature* probes)
{

  probes -> begin();
  Serial.print("Locating devices...");
  probes -> begin();
  Serial.print("Found ");
  Serial.print(probes -> getDeviceCount(), DEC);
  Serial.println(" devices.");
  return probes -> getDeviceCount();
}
float ds18b20Read(DeviceAddress deviceAddress,DallasTemperature* probes)
{
    //Serial.print("Requesting temperatures...");
    probes->requestTemperatures(); // Send the command to get temperatures
    //Serial.println("DONE");
    delay(10);
    float tempC = probes->getTempC(deviceAddress);
    if(tempC == DEVICE_DISCONNECTED_C) 
    {
        return -127.0;
    }
    return tempC;
} 