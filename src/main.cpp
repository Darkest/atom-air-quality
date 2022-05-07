#include <Arduino.h>
#include "M5Atom.h"
#include "utility/Power.h"
#include "Adafruit_SGP30.h"
//#include "Arduino_JSON.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "bottoken.cpp"
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

// Wifi network station credentials
#define WIFI_SSID "ROSSIA BEZ PUTINA_2g"
#define WIFI_PASSWORD "Tassadar"

uint8_t nPix = 0;
CRGB RED = CRGB(255, 0,0);
CRGB GREEN = CRGB(0, 255,0);
Adafruit_SGP30 sgp;
POWER power;

const long sensor_interval = 5000;
const long bot_interval = 5000;
unsigned long previousMillis = 0;
unsigned long previousMillisBot = 0;
unsigned long currentMillis = 0;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;  

extern char greenRect[];

void connectToWifi(){
  
}

void setup() {
  M5.begin(true, false, true); 
  setCpuFrequencyMhz(80);
  //esp_sleep_enable_timer_wakeup(min(sensor_interval, bot_interval)*1000);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  if (!sgp.begin()){  //Init the sensor
    Serial.println("Sensor not found");
    while (1);
  }

  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.softAPIP
  // Add root certificate for api.telegram.org
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  // secured_client.setInsecure();
  uint8_t n = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    M5.dis.setWidthHeight(5,15);
    M5.dis.displaybuff((uint8_t *)greenRect, 0, n);
    n=n+5;
    delay(200);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  bool wifiSleepEn = WiFi.setSleep(true);
  Serial.printf("WiFi sleep was enabled: %s\r\n", wifiSleepEn ? "true" : "false");
  //configTime(0, 0, "pool.ntp.org");  
}

void handleNewMessages() {
  M5.dis.drawpix(nPix, GREEN);
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  Serial.printf("new msgs count:%d\n\rlast_i:%ld\n\r", numNewMessages, bot.last_message_received);
  for (int i = 0; i < numNewMessages; i++)
  {
    String h2raw = String("rawH2=") + sgp.rawH2 + "\n\r";
    String ethanolRaw = String("rawEthanol=") + sgp.rawEthanol + "\n\r";
    String tvoc = String("TVOC=") + sgp.TVOC + "\n\r";
    String eco2 = String("eCO2=") + sgp.eCO2 + "\n\r";
    String response = h2raw + ethanolRaw + tvoc + eco2;

    bot.sendMessage(bot.messages[i].chat_id, response);
  }
  M5.dis.drawpix(nPix, RED);
}

void loop() {
  M5.dis.clear();
  M5.dis.drawpix(nPix, RED);
  currentMillis = millis();
  if (currentMillis - previousMillis >= sensor_interval) {
        previousMillis = currentMillis;

        if (!sgp.IAQmeasure()) {
            Serial.println("Measurement failed");
            return;
        }

        if (!sgp.IAQmeasureRaw()) {
            Serial.println("Raw Measurement failed");
            return;
        }

        Serial.printf("rawH2=%d\n\r", sgp.rawH2);
        Serial.printf("rawEthanol=%d\n\r", sgp.rawEthanol);
        Serial.printf("TVOC=%d\n\r", sgp.TVOC);
        Serial.printf("eCO2=%d\n\r", sgp.eCO2);
        }
        
  if (currentMillis - previousMillisBot >= bot_interval) {
    previousMillisBot = currentMillis;
    handleNewMessages();
  }
      
  nPix = (nPix + 1) % 25; 
  Serial.flush();
  //esp_err_t err = esp_light_sleep_start();
  delay(200);
}