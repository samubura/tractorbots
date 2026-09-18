#define LARGE_JSON_BUFFERS 1
#define USE_COLOR_SENSOR true
#define USE_IRRIGATION false
#define USE_LIDAR false
#define ROBOT_NAME "R5-D4"
#define ROBOT_COLOR RED
#define TEXT_COLOR WHITE
#define LOOP_DELAY 10

#define PIN_BAT_VOLTS 35
#define PIN_BAT_AMPS 36
#define PIN_LIDAR_ENABLE 5
#define PIN_TEMPERATURE_SENSOR 19

#include <Arduino.h>

#include<M5Stack.h>
//#include "sdk/include/angles.h"
#include "doProcess.h"
#include "mapData.h"
#include "X2driver.h"
//#include "lidarcar.h"
#include "lock.h"
//#include "espHttpServer.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "TD.h"
#include "Robot.h"

#if USE_COLOR_SENSOR
#include "Adafruit_TCS34725.h"
#endif

//-------------- 
constexpr char WIFI_SSID[] = "PSLab-TestNet";
constexpr char WIFI_PASS[] = "psl4b.test";

boolean wheelMoving = false;
long startedMoving = 0;
int requestedDuration = 0;

long totalDuration = 0;

float temperature = 28.0;
long lastUpdate = 0;
int updateInterval = 1000;
float batvol = 11.1;
float current = 0.0;

#if USE_LIDAR
float lastLidarStash[720] = { 0.0 };
String lidarData = "[]";
#endif


#if USE_IRRIGATION
boolean irrigating = false;
long startedIrrigation = 0;
int requestedIrrigationDuration = 0;
int waterLevel = 10;
#endif

//-------------------------- objects ------------------------
Robot robot;
AsyncWebServer server(80);

#if USE_LIDAR
X2 lidar;
#endif

#if USE_COLOR_SENSOR
Adafruit_TCS34725 colorsensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
#endif
//-------------------------- methods -----------------------

void lcdPrint(String str); //for debug and messages

void initWiFi();
void initServer();

void refreshDisplay(); //display all data

void doWheelControl(const JsonVariant &input);

#if USE_COLOR_SENSOR
void initColorSensor();
String doSoilCheck();
#endif


#if USE_IRRIGATION
void drawWaterLevel(int waterLevel);
void doIrrigation(const JsonVariant &input);
#endif

#if USE_LIDAR
void createLidarData();
void displayMap();
static void dis_task(void *arg);
#endif

//-----------------------------------------------------------

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {
  M5.begin(true, false, false);
  robot.begin();
  M5.Speaker.begin();
  lcdPrint("Waiting network");
  initWiFi();
  initServer();

  #if USE_COLOR_SENSOR
  lcdPrint("Check color sensor");
  initColorSensor();
  #endif

  #if USE_LIDAR
  //Create separate task to display map on LCD
  xTaskCreatePinnedToCore(dis_task, "lidar", 10 * 1024, NULL, 1, NULL, 1); 
  #endif

  //when everything ok display the ok values
  refreshDisplay();
}

void loop() {

  if(wheelMoving && (millis() - startedMoving) > requestedDuration){
    robot.wheelCommand(0,0,0,0);
    wheelMoving = false;
  }
  
  if(millis() - lastUpdate > updateInterval){
    batvol = robot.getBatteryVoltage(); 
    current = robot.getTotalCurrent();    
    lastUpdate = millis();
  }
  
  #if USE_IRRIGATION
  if(irrigating && (millis() - startedIrrigation) > requestedIrrigationDuration){
    robot.ledCommand(0,0,0,0);
    irrigating = false;
  }
  #endif

  #if USE_LIDAR
  while (Serial1.available()) {
    lidar.lidar_data_deal(Serial1.read());
  }
  #endif

  M5.Speaker.update();
  
  delay(LOOP_DELAY);
}

//###################  WIFI  ############################

void initWiFi() {
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.printf("Connecting to %s .", WIFI_SSID);
    while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(200); }
    Serial.println(" ok");

    IPAddress ip = WiFi.localIP();

    Serial.printf("SSID: %s\n", WIFI_SSID);
    Serial.printf("Channel: %u\n", WiFi.channel());
    Serial.printf("IP: %u.%u.%u.%u\n", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, ip >> 24); 
}

//###################  WEBSERVER  ############################
void initServer()
{
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
      Serial.println("GET / request");
      request->send(200, "application/json", td); 
      //TODO best to apply the correct IP at runtime to the TD instead of doing it manually...
  });
    
  //-------------------------------- Properties ----------------

  server.on("/properties/batteryvoltage", HTTP_GET, [](AsyncWebServerRequest *request){ 
    Serial.println("GET /batteryvoltage "); 
    request->send(200, "application/json", String(batvol)); 
  });

  server.on("/properties/motorcurrent", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("GET /motorcurrent "); 
    request->send(200, "application/json", String(current)); 
  });

  server.on("/properties/temperature", HTTP_GET, [](AsyncWebServerRequest *request){ 
    Serial.println("GET /temperature "); 
    request->send(200, "application/json", String(temperature)); 
  });

  #if USE_COLOR_SENSOR
  server.on("/properties/soilcondition", HTTP_GET, [](AsyncWebServerRequest *request){ 
      Serial.println("GET /soilcondition "); 
      String colorData = doSoilCheck();

      request->send(200, "application/json", colorData); }
    );
  #endif

  #if USE_IRRIGATION
    server.on("/properties/waterlevel", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("GET /waterlevel "); 
    request->send(200, "application/json", String(waterLevel)); 
  });
  #endif
  
  #if USE_LIDAR
  server.on("/properties/lidar", HTTP_GET, [] (AsyncWebServerRequest *request) {
      createLidarData();
      request->send(200, "application/json", lidarData);
  }); 
  #endif
  
  //------------------------------ Actions -------------------------------
  AsyncCallbackJsonWebHandler *wheelHandler = new AsyncCallbackJsonWebHandler("/actions/wheelcontrol", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<200> data;
    data = json.as<JsonObject>();
    doWheelControl(json);
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
  });
  server.addHandler(wheelHandler); 

  #if USE_IRRIGATION
  AsyncCallbackJsonWebHandler *waterHandler = new AsyncCallbackJsonWebHandler("/actions/irrigate", [](AsyncWebServerRequest *request, JsonVariant &json) {
      StaticJsonDocument<200> data;
      data = json.as<JsonObject>();
      int duration = data["duration"];
      if(duration <= 5 && waterLevel >= duration)
      {
        doIrrigation(json);     
        request->send(200, "application/json", "true");
      }else{
        request->send(400, "application/json", "false");
      }
  });
  server.addHandler(waterHandler); 

  server.on("/actions/refillwater", HTTP_POST, [] (AsyncWebServerRequest *request) {
      waterLevel = 10; 
      refreshDisplay();   
      request->send(200, "application/json", "true");
      M5.Speaker.tone(1333, 100);
  });  
  #endif

  server.begin(); 
}

//###################  DISPLAY FUNCTIONS  ############################

void lcdPrint(String str) {
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(3, 10);
  M5.Lcd.println(str);
}

void refreshDisplay() {
  M5.Lcd.clear(ROBOT_COLOR);
  M5.Lcd.fillScreen(ROBOT_COLOR);
  M5.Lcd.setTextColor(TEXT_COLOR);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(3, 10);
  M5.Lcd.println(ROBOT_NAME);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println(WiFi.localIP());

  #if USE_IRRIGATION
  drawWaterLevel(waterLevel);
  #endif
}

#if USE_IRRIGATION
void drawWaterLevel(int waterLevel) {
  M5.Lcd.drawRect(3, 150, M5.Lcd.width()-6, 15, WHITE);
  if(waterLevel == 10){
    M5.Lcd.fillRect(3, 150, M5.Lcd.width()-6, 15, WHITE);
  } else {
    int width = ((M5.Lcd.width() - 6 ) / 10) * waterLevel;
    M5.Lcd.fillRect(3, 150, width, 15, WHITE);
  }
  M5.Lcd.setCursor(3, 130);
  M5.Lcd.print("Water Level: ");
  M5.Lcd.print(waterLevel);
}
#endif


//###################  MOVEMENT  ############################

void doWheelControl(const JsonVariant &input) {
  JsonObject inputObj = input.as<JsonObject>();
  int axis = inputObj["axis"];
  int spd = inputObj["speed"];
  int duration = inputObj["duration"];
  
  int wa = 0, wb = 0, wc = 0, wd = 0;
  if(axis == 0)
  { //X-axis == move back and forth
    wa = spd;
    wb = spd;
    wc = spd;
    wd = spd;
  }else if(axis == 1)
  { //Y-axis == move sideways
    wa = spd;
    wb = -1*spd;
    wc = -1*spd;
    wd = spd;
  }else if(axis == 2)
  { //Z-axis == rotate on itself
    wa = spd;
    wb = -1*spd;
    wc = spd;
    wd = -1*spd;
  }
    
  if(duration > 0){
    robot.wheelCommand(wa, wb, wc, wd);
    requestedDuration = duration;
    wheelMoving = true;
    startedMoving = millis();
  }else{
    wheelMoving = false;
    robot.wheelCommand(0,0,0,0);
  }
  //temperature = temperature + (duration/1000);
  //batvol = batvol - (duration * .00001);
}


//###################  COLOR SENSOR  ############################

#if USE_COLOR_SENSOR
void initColorSensor(){
  while(!colorsensor.begin()){
      Serial.println("No TCS34725 found ... check your connections");
      lcdPrint("Color sensor not found!!");
      delay(1000);
  }
  colorsensor.setIntegrationTime(TCS34725_INTEGRATIONTIME_154MS);
  colorsensor.setGain(TCS34725_GAIN_4X);  
}


String doSoilCheck(){
  uint16_t clear, red, green, blue; 
  colorsensor.getRawData(&red, &green, &blue, &clear);
  StaticJsonDocument<200> soil;
  String colorData = "";
  soil.clear();
  soil["ph"] = clear;
  soil["moisture"] = blue;
  soil["density"] = red;
  soil["nitrate"] = green;
  serializeJson(soil, colorData);
  M5.Speaker.tone(440, 100);
  return colorData;
}

#endif

//##################  IRRIGATION  ###########################

#if USE_IRRIGATION
void doIrrigation(const JsonVariant &input) {
  JsonObject inputObj = input.as<JsonObject>();
  int duration = inputObj["duration"];
  if(duration > 0){
    robot.ledCommand(0,0,0,0xff);
    requestedIrrigationDuration = duration * 1000;
    irrigating = true;
    startedIrrigation = millis();
    waterLevel = waterLevel - duration;
    refreshDisplay();
  }else{
    irrigating = false;
    robot.ledCommand(0,0,0,0);
  }  
}
#endif

//###################  LIDAR  ############################

#if USE_LIDAR
void createLidarData(){
  float map_data_stash[720] = { 0.0 };
  xSemaphoreTake( xSemaphore, portMAX_DELAY);
  memcpy(map_data_stash, lidar.dismap.mapdata, 720*sizeof(float));
  xSemaphoreGive( xSemaphore );
  String ldata = "[" ;
  for (int i = 0; i < 720; i++) {
      ldata = ldata + String((int)map_data_stash[i]);
      if(i < 719)
        ldata = ldata + ",";
  }
  lidarData = ldata + "]";
}

void displayMap(void) {
  float map_data_stash[720] = { 0.0 };
  xSemaphoreTake( xSemaphore, portMAX_DELAY);
  memcpy(map_data_stash, lidar.dismap.mapdata, 720*sizeof(float)); 
  xSemaphoreGive( xSemaphore );

  for (int i = 0; i < 720; i++) {
      float oldAng = from_degrees((i / 2.0));
      float Ang = from_degrees((i / 2.0));
      float oldX = (sin(oldAng) * lidar.oldmap.mapdata[i]/5);
      float oldY = (cos(oldAng) * lidar.oldmap.mapdata[i]/5);
      float X = (sin(Ang) * map_data_stash[i]/5);
      float Y = (cos(Ang) * map_data_stash[i]/5);

      if (lidar.oldmap.mapdata[i] > 0) {
        M5.Lcd.drawPixel(-oldX + 160, oldY + 120, BLACK);  
      }

      if (map_data_stash[i] > 0) {
        M5.Lcd.drawPixel(-X + 160, Y + 120, WHITE);  
      }
      lidar.oldmap.mapdata[i] = map_data_stash[i];
  }
}


static void dis_task(void *arg) {
  while(1) {
    //Serial.printf("uart_task\r\n");
    if (lidar.disPlayFlag){
      displayMap();
      delay(5); 
      lidar.disPlayFlag = 0;
    }    
    delay(10);
  }
}
#endif


