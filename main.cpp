#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <SHT1x.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 0
#define dataPin  16
#define clockPin 17
SHT1x sht1x(dataPin, clockPin);
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

extern "C" {
  #include "esp_system.h"
  #include <driver/adc.h>
  uint8_t temprature_sens_read(); //include idf
}



String host = "labsistemas.javerianacali.edu.co:8002";

String apiKeySoil = "G7RYW1XF9ED3ECAH";
String apiKeyAir = "4ELO3TBBFNITUNUL";
String apiKeyInter = "F3G1EZRNC2HN9F8O";

const char* ssid = "ciat-visitors";
const char* password =  "javeriana1";
const int sleepTimeS = 30;

const int inpVH400 = 35;
const int inpPar =33;
const int inpWind = 32;
const int inpMeth = 34;
//const int in
/* LED pin */
byte ledPin = 5;

float vmc;
float temp1;
float par;
float temp_c;
//float temp_f;
float humidity;


String windDir = "";

#define WIND_DIR_PIN 36

//methane variables

float R0 = 11.820; //Sensor Resistance in fresh air from
float m = -0.318; //Slope
float b = 1.133; //Y-Intercept

void windDirCalc(int vin)
{
  if      (vin < 150) windDir="202.5";
  else if (vin < 300) windDir = "180";
  else if (vin < 400) windDir = "247.5";
  else if (vin < 600) windDir = "225";
  else if (vin < 900) windDir = "292.5";
  else if (vin < 1100) windDir = "270";
  else if (vin < 1500) windDir = "112.5";
  else if (vin < 1700) windDir = "135";
  else if (vin < 2250) windDir = "337.5";
  else if (vin < 2350) windDir = "315";
  else if (vin < 2700) windDir = "67.5";
  else if (vin < 3000) windDir = "90";
  else if (vin < 3200) windDir = "22.5";
  else if (vin < 3400) windDir = "45";
  else if (vin < 4000) windDir = "0";
  else windDir = "0";
}



float readVH400(int inpVH400) {


  // Read value and convert to voltage
  int lecturaVH400 = analogRead(inpVH400);
  float sensorVoltage = lecturaVH400*(3.0 / 4096.0);
  float VWC;

  // Calculate VWC
  if(sensorVoltage <= 1.1) {
    VWC = 10*sensorVoltage;
  } else if(sensorVoltage > 1.1 && sensorVoltage <= 1.3) {
    VWC = 25*sensorVoltage-17.5;
  } else if(sensorVoltage > 1.3 && sensorVoltage <= 1.82) {
    VWC = 48.08*sensorVoltage-47.5;
  } else if(sensorVoltage > 1.82) {
    VWC = 26.32*sensorVoltage-7.89;
  }
  return(VWC);
}



void setup() {
  Serial.begin(115200);
 sensors.begin();
  pinMode(inpVH400,INPUT);
  pinMode(inpMeth,INPUT);
  pinMode(inpPar,INPUT);
  pinMode(inpWind,INPUT);
  pinMode(ledPin, OUTPUT);

  sensors.begin();
  Serial.begin(115200);
  delay(2000);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  digitalWrite(ledPin, HIGH); //status WiFi

}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    digitalWrite(ledPin, HIGH);
    delay(10);
    digitalWrite(ledPin, LOW);
    HTTPClient http;

    float vmc = readVH400(inpVH400);
    uint8_t temp = temprature_sens_read();
    float temp1 = (temp-32)/1.8;

    //Methane linear
    float lectura = analogRead(inpMeth);
    float voltMeth = lectura * (3.0 / 4096.0);
    float ratio = (((5.0 * 10.0) / voltMeth) - 10.0)/R0;
    double ppm_log = (log10(ratio) - b) / m; //Get ppm value in linear scale
    double ppm = pow(10, ppm_log)/ 10000;

    // PAR sensor
    float parLectura = analogRead(inpPar);
    float voltPar = parLectura * (3.0 /4096.0);
    float par  = voltPar * 5000; //(4000/0.8)

    //Wind sensor reading
    float windLectura = analogRead(inpWind);
    float voltWind = windLectura *(3.0/4096);
    float windSpeed = 32.4 * (voltWind-0.4)/1.6;

    //windDirCalc
    windDirCalc(analogRead(WIND_DIR_PIN));

    //humidity and Temperature
    float temp_c;
    //float temp_f;
    float humidity;

    // Read values from the sensor
    temp_c = sht1x.readTemperatureC();
    //temp_f = sht1x.readTemperatureF();
    humidity = sht1x.readHumidity();
    //DS18B20
    sensors.requestTemperatures();
    float tempSoil = sensors.getTempCByIndex(0);

    Serial.print("Temperature soil is: ");
    Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?


    Serial.print("Methane ppm:");
    Serial.println(ppm);

    Serial.print("VMC :");
    Serial.println(vmc);

    Serial.print("temp internal :");
    Serial.println(temp1);
    Serial.print("wind :");
    Serial.println(windSpeed);

    Serial.print("Temperature: ");
    Serial.print(temp_c, DEC);
    Serial.print("C / ");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    sensors.requestTemperatures();

    String updateURL = "http://" + host + "/update?api_key=" + apiKeySoil +"&field1="+String(vmc)+"&field2="+String(sensors.getTempCByIndex(0));
    String updateURL2 = "http://" + host + "/update?api_key=" + apiKeyInter +"&field1="+String(temp1);//segundo canal
    String updateURL3 = "http://" + host + "/update?api_key=" + apiKeyAir +"&field1="+String(ppm)+"&field7="+String(par)+"&field4="+String(windSpeed);
           updateURL3 +="&field3="+String(humidity)+"&field2="+String(temp_c)+"&field5="+String(windDir)+"&field6="+String("0");

    http.begin(updateURL);

    //http.GET()
    volatile int httpCode = http.GET();                                        //Make the request

    if (httpCode > 0) { //Check for the returning code

        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }

    else {
      Serial.println("Error on HTTP request");
    }



    http.end();

    http.begin(updateURL2); //Specify the URL

    http.GET();
                                       //Make the request
                                        //Make the request

    if (httpCode > 0) { //Check for the returning code

        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }

    else {
      Serial.println("Error on HTTP request");
    }


    http.end(); //Free the resources

    http.begin(updateURL3); //Specify the URL

    http.GET();                                        //Make the request

    if (httpCode > 0) { //Check for the returning code

        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }

    else {
      Serial.println("Error on HTTP request");
    }


    http.end();


  }else{

     Serial.println("Error in WiFi connection");

  }

   delay(10000);  //Send a request every 10 seconds

   Serial.println("deepsleep empieza");
   ESP.deepSleep(sleepTimeS * 1000000);
   Serial.println("Deep end");

}
