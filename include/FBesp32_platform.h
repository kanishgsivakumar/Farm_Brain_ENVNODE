#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <PubSubClient.h>

extern uint32_t getUniqueID();
extern bool Wificonnect();
extern void MQTTconnect(const char* mqtt_server,const uint16_t mqtt_port,PubSubClient* client,char* chipId_s);
extern void MQTTreconnect(PubSubClient* client,char* chipId_s);
extern void Otastart(AsyncWebServer* otaServer,char* chipId_s);
