#include <Arduino.h>
#include <SHT1x.h> //id_library 86
#include <Wire.h>
#include <ESP8266WiFi.h>

// replace with your channel's thingspeak API key,

String apiKey = "O64EJ5WZ0Y0MVNY9";

// replace with your wifi ssid and wpa2 key
//const char *ssid =  "Movistar_24000007";

const char *ssid =  "Movistar_24000007";
const char *pass =  "00914835682";
const char* server = "api.thingspeak.com";

// Specify data and clock connections and instantiate SHT1x object

#define clockPin 5 //D1
#define dataPin  4 //D2
#define dataPin2 2 //D4

//Create instances of the objects

SHT1x sht1x(dataPin, clockPin);
SHT1x sht1x2(dataPin2, clockPin);


WiFiClient client;


void setup()
{
   Serial.begin(115200); // Open serial connection to report values to host
   delay(10);

Serial.print("Connecting to ");
Serial.println(ssid);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);

while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

Serial.println("");
Serial.println("WiFi connected");

}


void loop()
{
  // Read values from the sensor

  float t1 = sht1x.readTemperatureC();
  float t2 = sht1x2.readTemperatureC();
  float h1 = sht1x.readHumidity();
  float h2 = sht1x2.readHumidity();

if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com

  String postStr = apiKey;

         postStr +="&field1=";
         postStr += String(t1);

         postStr +="&field2=";
         postStr += String(h1);

         postStr +="&field3=";
         postStr += String(t2);

         postStr +="&field4=";
         postStr += String(h2);

         postStr += "\r\n\r\n";


   client.print("POST /update HTTP/1.1\n");
   client.print("Host: api.thingspeak.com\n");
   client.print("Connection: close\n");
   client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
   client.print("Content-Type: application/x-www-form-urlencoded\n");
   client.print("Content-Length: ");
   client.print(postStr.length());
   client.print("\n\n");
   client.print(postStr);

   //print in serial monitor

   Serial.print("Temperature1: ");
   Serial.print(t1, DEC);
   Serial.print("C / ");
   Serial.print("Temperature2: ");
   Serial.print(t2, DEC);
   Serial.print("C / ");
   Serial.print("Humidity1: ");
   Serial.print(h1);
   Serial.println("");
   Serial.print("Humidity2: ");
   Serial.print(h2);

   Serial.println("%. Send to Thingspeak.");

}

client.stop();
Serial.println("Waiting...");

// thingspeak needs minimum 15 sec delay between updates, set 30 seconds
delay(30000);
}

/*
SHT sensor
red         vcc
yellow      sck
green       gnd
blue        data

10k resistor between data pin and vcc

*/
