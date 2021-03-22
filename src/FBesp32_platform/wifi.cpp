#include <Arduino.h>
#define _ESPASYNC_WIFIMGR_LOGLEVEL_  4
#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager


bool Wificonnect()
{
    AsyncWebServer webServer(80);
    DNSServer dnsServer;
    Serial.print("\nStarting Async_AutoConnect_ESP32_minimal on " + String(ARDUINO_BOARD)); Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer,"Farm-Brain" );
    //ESPAsync_wifiManager.resetSettings();   //reset saved settings
    ESPAsync_wifiManager.setAPStaticIPConfig(IPAddress(192,168,132,1), IPAddress(192,168,132,1), IPAddress(255,255,255,0));
    ESPAsync_wifiManager.autoConnect("Farm_Brain","00000000");
    if (WiFi.status() == WL_CONNECTED) 
    {
        Serial.print(F("Connected. Local IP: ")); Serial.println(WiFi.localIP());
        return true;
    }
    else 
    {
        Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));
        return false;
    }
}