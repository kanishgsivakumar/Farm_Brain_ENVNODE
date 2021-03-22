
/***
 * 
 * Unique ID 
 * 
 * This function generates UID from last 3 bytes of MAC address of the ESP32
 * which 32 bits in lenght and returns. 
 * 
 * created on 12-03-2021 by Kanishg
 * referred from GetChipID example from arduino-esp32 by cweinhofer
 ***/
#include <Arduino.h>

uint32_t getUniqueID()
{
    uint32_t chipId = 0;

    for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
    return chipId;
}