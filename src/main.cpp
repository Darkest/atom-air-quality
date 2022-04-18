#include <Arduino.h>
#include "M5Atom.h"
#include "Adafruit_SGP30.h"
//#include "Arduino_JSON.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Wifi network station credentials
#define WIFI_SSID "ROSSIA BEZ PUTINA_2g"
#define WIFI_PASSWORD "Tassadar"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

uint8_t nPix = 0;
CRGB colPix = CRGB(255, 0,0);
Adafruit_SGP30 sgp;

const long interval = 5000;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;  

extern char greenRect[2+75];
extern unsigned char AtomImageData[375 + 2];

void setup() {
  M5.begin(true, false, true); 

   if (!sgp.begin()){  //Init the sensor. 初始化传感器
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
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
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
}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
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

  M5.dis.clear();
  M5.dis.drawpix(nPix, colPix);
  nPix = (nPix + 1) % 25; 
  delay(200);
}