#include <Arduino.h>
#include <FBesp32_platform.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FBenv_sensor.h>
#include <time.h>
#define ONE_WIRE_BUS 23
AsyncWebServer otaServer(80);

const char* mqtt_server = "test.mosquitto.org";
const uint16_t mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
extern int interval = 10;


StaticJsonDocument<512> packet;
char data[512];

uint32_t chipId = getUniqueID();
char chipId_s[20] ;

Adafruit_BME280 bme1,bme2; 

BH1750 lightMeter;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
DeviceAddress outsideThermometer;
float probe_temp = 0;
unsigned long previousMillis = 0;
int counter = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 5.5*3600;
const int   daylightOffset_sec = 0;
struct tm timeinfo;
int prevHour= 0;
int led_wifi = 25;
int led_server = 33;
void setup() {
	Serial.begin(9600);
  pinMode(led_wifi,OUTPUT);
  pinMode(led_server,OUTPUT);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(led_wifi,LOW);
    Wificonnect();
    delay(1000);
  }
  digitalWrite(led_wifi,HIGH);
  delay(2500);
  digitalWrite(led_server,LOW);
  MQTTconnect(mqtt_server,mqtt_port,&client,itoa(chipId,chipId_s,10));
  digitalWrite(led_server,HIGH);
  Otastart(&otaServer,itoa(chipId,chipId_s,10));
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (bme280Init(&bme1,0x76))
  Serial.println("BME 0x76 Initialized");

  if (bme280Init(&bme2,0x77))
  Serial.println("BME 0x77 Initialized");

  bh1750Init(&lightMeter,21,22);
  ds18b20Init( &sensors);
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  delay(1000);
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
    sensors.setResolution(insideThermometer, 9);
  Serial.println("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();

  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 0"); 
  delay(1000);
  Serial.print("Device 1 Address: ");
  printAddress(outsideThermometer);
    sensors.setResolution(outsideThermometer, 9);
  Serial.println("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(outsideThermometer), DEC); 
  Serial.println();

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
}

void loop() {
  client.loop();
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(led_wifi,LOW);
    digitalWrite(led_server,LOW);
    Wificonnect();
    delay(1000);
  }
  digitalWrite(led_wifi,HIGH);

  if (!client.connected()) {
    digitalWrite(led_server,LOW);
    MQTTreconnect(&client,itoa(chipId,chipId_s,10));
    delay(1000);
  }
  digitalWrite(led_server,HIGH);
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  char timeHour[3],timeMin[3];
  strftime(timeHour,3, "%H", &timeinfo);
  strftime(timeMin,3, "%M", &timeinfo);
  if (atoi(timeHour)>prevHour)
  {
    counter = 0;
    prevHour= atoi(timeHour);
  }
  packet["IP"] =  WiFi.localIP().toString();
  packet["UID"] = getUniqueID();
  packet["Hours"]= timeHour;
  packet["counter"] = counter;
  packet["Humidity1"] = bme1.readHumidity();
  packet["Temperature1"] = bme1.readTemperature();
  packet["Pressure1"] = bme1.readPressure()/100.0F;
  packet["Humidity2"] = bme2.readHumidity();
  packet["Temperature2"] = bme2.readTemperature();
  packet["Pressure2"] = bme2.readPressure()/100.0F;
  probe_temp = ds18b20Read(insideThermometer,&sensors);
  while ((probe_temp == 85)or(probe_temp == -127.0))
  {
    Serial.print("Sensor1 "); Serial.println(probe_temp);
    ds18b20Init( &sensors);
    delay(50);
    probe_temp = ds18b20Read(insideThermometer,&sensors);
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval*500)
      break;
  
  }
  packet["Probe_Temperature1"] = probe_temp;
    probe_temp = ds18b20Read(outsideThermometer,&sensors);
  while ((probe_temp == 85)or(probe_temp == -127.0))
  {
    Serial.print("Sensor2 "); Serial.println(probe_temp);
    ds18b20Init( &sensors);
    delay(50);
    probe_temp = ds18b20Read(outsideThermometer,&sensors);
    Serial.println(".");
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval*500)
      break;
  
  }
  packet["Probe_Temperature2"] = probe_temp;
  packet["Light"] = lightMeter.readLightLevel();
  serializeJson( packet,  &data,  512);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval*1000){
    Serial.println(data);
    if (client.publish("/outtopic",data))
    {
      Serial.println("Published");
      counter++;
    }
    else
    {
      Serial.println("Error");
      counter++;
    }
    previousMillis =millis();
  }

    

  



}
