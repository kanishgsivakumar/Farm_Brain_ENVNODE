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

const char* mqtt_server = "mqtt.sustenance.co.in";
const uint16_t mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
int interval = 10;

StaticJsonDocument<768> packet;
char datastr[768];


uint32_t chipId = getUniqueID();
char chipId_s[10] ;
char outtopic[20] = "node/";
Adafruit_BME280 bme1,bme2;
bool bme1_status,bme2_status; 

BH1750 lightMeter;
bool lightMeter_status;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer[2];
int ds18b20_count;
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
  itoa(chipId,chipId_s,10);
  strcat(outtopic,chipId_s);
  strcat(outtopic,"/data");
  Serial.println(outtopic);
  MQTTconnect(mqtt_server,mqtt_port,&client,itoa(chipId,chipId_s,10));
  client.publish(outtopic,strcat(chipId_s ," is connected and Ready"));
  digitalWrite(led_server,HIGH);
  Otastart(&otaServer,itoa(chipId,chipId_s,10));
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  bme1_status= bme280Init(&bme1,0x76);
  if (bme1_status)
  Serial.println("BME 0x76 Initialized");

  bme2_status= bme280Init(&bme2,0x77);
  if (bme2_status)
  Serial.println("BME 0x77 Initialized");

  lightMeter_status =  bh1750Init(&lightMeter,21,22);
  if (lightMeter_status)
  Serial.println("BH1750 Initialized");

  ds18b20_count =  ds18b20Init( &sensors);
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) 
    Serial.println("ON");
  else
    Serial.println("OFF");
  for(int i = 0 ;i<ds18b20_count;i++)
  {
    if (!sensors.getAddress(thermometer[i], i)) 
    {
      Serial.print("Unable to find address for Device "); 
      Serial.println(i);
    }
    else
    {
      delay(1000);
      Serial.print("Device");
      Serial.print(i);
      Serial.print(" Address:");
      printAddress(thermometer[i]);
      sensors.setResolution(thermometer[i], 9);
      Serial.print("Device");
      Serial.print(i);
      Serial.print(" Resolution:");
      Serial.print(sensors.getResolution(thermometer[i]), DEC); 
      Serial.println();
    }

  }


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
    client.publish(outtopic,"connected and ready");
  }
  digitalWrite(led_server,HIGH);
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  char timeHour[3],timeMin[3];
  strftime(timeHour,3, "%H", &timeinfo);
  strftime(timeMin,3, "%M", &timeinfo);
  packet["UID"] = getUniqueID();
  if (atoi(timeHour)!=prevHour)
  {
    counter = 0;
    prevHour= atoi(timeHour);
  }
  JsonArray data = packet.createNestedArray("data");
  JsonObject data_0 = data.createNestedObject();
  data_0["key"] = "counter";
  data_0["value"] = counter;

  JsonObject data_1 = data.createNestedObject();
  data_1["key"] = "hours";
  data_1["value"] = timeHour;
  if (bme1_status)
  {
    JsonObject data_2 = data.createNestedObject();
    data_2["key"] = "humidity1";
    data_2["value"]  = bme1.readHumidity();
    JsonObject data_4 = data.createNestedObject();
    data_4["key"] = "temperature1";
    data_4["value"] = bme1.readTemperature();
    JsonObject data_6 = data.createNestedObject();
    data_6["key"] = "pressure1";
    data_6["value"] = bme1.readPressure()/100.0F;

  }
  if (bme2_status)
  {
    JsonObject data_3 = data.createNestedObject();
    data_3["key"] = "humidity2";
    data_3["value"]  = bme2.readHumidity();
    JsonObject data_5 = data.createNestedObject();
    data_5["key"] = "temperature2";
    data_5["value"] = bme2.readTemperature();
    JsonObject data_7 = data.createNestedObject();
    data_7["key"] = "pressure2";
    data_7["value"] = bme2.readPressure()/100.0F;

  }
  for(int i = 0 ;i<ds18b20_count;i++)
  {
    probe_temp = ds18b20Read(thermometer[i],&sensors);
    while ((probe_temp == 85)or(probe_temp == -127.0))
    {
      Serial.print("Sensor ");Serial.print(i); Serial.println(probe_temp);
      ds18b20Init( &sensors);
      delay(50);
      probe_temp = ds18b20Read(thermometer[i],&sensors);
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval*500)
        break;
    
    }
    if (i == 0)
    {
      JsonObject data_8 = data.createNestedObject();
      data_8["key"] = "probetemperature1";
      data_8["value"] = probe_temp;
    }
    if (i == 1)
    {
      JsonObject data_9 = data.createNestedObject();
      data_9["key"] = "probetemperature2";
      data_9["value"] = probe_temp;
    }
  }
  if(lightMeter_status)
  {
    JsonObject data_10 = data.createNestedObject();
    data_10["key"] = "lux";
    data_10["value"] = 29.2;  
  } 
  serializeJson(packet, &datastr, 768);
  packet.clear();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval*1000){
    Serial.println(datastr);
    if (client.publish(outtopic,datastr))
    {
      Serial.print("Published to ");
      Serial.println(outtopic);
      counter++;
      previousMillis =millis();
    }
    else
    {
      Serial.println("Error");
      counter++;
    }
  }
}
