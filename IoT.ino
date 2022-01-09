#include <LiquidCrystal.h>
#include <Wire.h>
#include <SPI.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "index_html.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>


#define BUTTON_MINUS 35
#define BUTTON_PLUS 33
#define RELAY_AC 27
#define RELAY_HEATING 26
#define REF_TEMP_STEP 0.5
//#define HYSTERESIS 0.5

LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

float ref_temp=25.00, real_temp;
bool interrupt_flag = false;
float pressure = 0;

String desired_temp = "25";
const char* PARAM_INPUT = "value";

float HYSTERESIS = 0.50;

unsigned long currentTime = millis();
unsigned long previousTime = 0, lastIntTime; 
const long timeoutTime = 4000;

// Replace with your network credentials
const char* ssid     = "wi-fi-ssid";
const char* password = "fill-in";

AsyncWebServer server(80);


Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
sensors_event_t temp_event, pressure_event;


String readBMETemperature() {

    bmp_temp->getEvent(&temp_event);
    real_temp = temp_event.temperature;
    return String(real_temp);
  
}

String readBMEPressure() {
 
    bmp_pressure->getEvent(&pressure_event);
    pressure =  pressure_event.pressure;
    return String(pressure_event.pressure);
  }


void update_LCD(){
  lcd.clear();
  if(!interrupt_flag)
  {
    readBMETemperature();
    readBMEPressure();
    lcd.setCursor(0,0);
    lcd.print("Temp = ");
    lcd.print(real_temp);
    lcd.print(" *C");
    lcd.setCursor(0,1);
    lcd.print("Pres = ");
    lcd.print(pressure_event.pressure);
    lcd.print("hPa");
  }

  else {   
    lcd.setCursor(0,0);
    lcd.print("Set temp = ");
    lcd.print(ref_temp);
    lcd.print(" *C");
  }
}


String getHeatingState()
{

  if (!digitalRead(RELAY_HEATING)){
      return String("ON");
  }
 
  return String("OFF");
}


String getACState()
{

  if (!digitalRead(RELAY_AC)){
      return String("ON");
  }

  return String("OFF");
}

void IRAM_ATTR ISR_func()
{
  interrupt_flag = true;
  
}

// Replaces placeholder with BME values
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readBMETemperature();
  }
 if(var == "PRESSURE"){
    return readBMEPressure();
  }
 if(var =="HEATING_STATE") {

    return getHeatingState();
 }
 if(var =="AC_STATE") {

    return getACState();
 }
 if(var =="DESIRED_TEMP"){
    return String(ref_temp);
 }

  return String();
}


void setup() {
 // Serial port for debugging purposes
  Serial.begin(115200);
  lcd.begin(16, 2);
  update_LCD();

  pinMode(BUTTON_MINUS, INPUT);
  pinMode(BUTTON_PLUS, INPUT);
  attachInterrupt(BUTTON_MINUS, ISR_func, RISING);
  attachInterrupt(BUTTON_PLUS, ISR_func, RISING);
  
  pinMode(RELAY_HEATING, OUTPUT);
  pinMode(RELAY_AC, OUTPUT);
  digitalWrite(RELAY_AC, HIGH);
  digitalWrite(RELAY_HEATING, HIGH);
  
   // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());


   bool status;   
  Serial.println(F("BMP280 Sensor event test"));

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1) delay(10);
  }

   /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();


 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",readBMETemperature().c_str() , processor);
  });
  
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",readBMEPressure().c_str(),processor);
  });

  server.on("/heating", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",getHeatingState().c_str(),processor);
  });
  
   server.on("/ac", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",getACState().c_str(),processor);
  });
  server.on("/desired_temp", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      ref_temp = inputMessage.toFloat();
    }

      request->send_P(200, "text/plain", String(ref_temp).c_str(), processor);

  });


  server.on("/set_hist", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT)) {;
      inputMessage = request->getParam(PARAM_INPUT)->value();
      HYSTERESIS = inputMessage.toFloat();
    }
      request->send_P(200, "text/plain", "OK", processor);

  });
  
  // Start server
  server.begin();

}


void loop()
{

  currentTime = millis(); 
 
    if (interrupt_flag == true)
     { 
        if ((currentTime - lastIntTime)> 50 && interrupt_flag)
          {  lastIntTime = currentTime;
             if(digitalRead(BUTTON_PLUS))
                {
                   ref_temp+=REF_TEMP_STEP;

                }
             else if(digitalRead(BUTTON_MINUS))
                {
                  ref_temp-=REF_TEMP_STEP;
                }
        update_LCD();
        interrupt_flag = false;
   
          }
    }
    else
    {
        if(currentTime-previousTime>timeoutTime)
        {

            if(real_temp>= ref_temp + HYSTERESIS)
            {
              digitalWrite(RELAY_AC,LOW);
              digitalWrite(RELAY_HEATING,HIGH);
            }
            else if (real_temp <= ref_temp - HYSTERESIS)
            {
              digitalWrite(RELAY_HEATING,LOW);
              digitalWrite(RELAY_AC,HIGH);
            }

            else {

                  digitalWrite(RELAY_HEATING,HIGH);
                  digitalWrite(RELAY_AC,HIGH);     
            }
          update_LCD();
          previousTime = currentTime;
    
       
        }
    }
}
