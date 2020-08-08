/*
  ESP8266 Blink by Simon Peter
  Blink the blue LED on the ESP-01 module
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/
#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "cc1100_arduino.h"
#include "esp_arduino_spi.h"
#include "wifi_secrets.h"
#include <ArduinoOTA.h>
#include "modulations.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 1
#endif

cc1100::ESPArduinoSPI spi433;
cc1100::ESPArduinoSPI spi868;
cc1100::CC1100 cc433(spi433);
cc1100::CC1100 cc868(spi868);

AsyncWebServer server(80);

typedef struct {
  long len;
  uint8_t data[];
} data_to_send_t;

cc1100::Modulation *current_modulation;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // init serial Port for debugging
  Serial.begin(115200);
  spi433.begin(15);
  spi868.begin(16);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  // Wait for connection
  DBG("Waiting for wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  AsyncCallbackWebHandler &handler = server.on("/send", [](AsyncWebServerRequest *request){
    if (current_modulation == NULL) {
      current_modulation = cc1100::Modulation::make((char *)request->_tempObject);
      current_modulation->start_send(cc433);
      request->_tempObject = NULL;
      request->send(200, "text/plain", "started");
      return;
    }
    request->send(400, "text/plain", "already sending");
  });
  handler.onBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){

    if (request->_tempObject == NULL){
      request->_tempObject = calloc(total + 1, 1);
      if (request->_tempObject == NULL){
        return;
      }
      ((char*)request->_tempObject)[total] = 0;
    }
    memcpy(request->_tempObject + index, data, len);
  });
  server.on("/r", [](AsyncWebServerRequest *request){
    long r = strtol(request->arg("r").c_str(), 0, 16);
    uint8_t res = spi433.read_register(r);
    char buf[100];
    snprintf(buf, 100, "%02x",res & 0xff);
    request->send(200, "text/plain", buf);
  });
  server.begin();

  // init CC1101 RF-module and get My_address from EEPROM
  cc433.begin(4, 5, // gpio pins (dont use serial!)
    0x2, //set ISM Band 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz
    0x7,//set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb; 7 = OOK_raw
    0x0,//set channel
    3);//addr
  cc433.show_main_settings();             //shows setting debug messages to UART
  cc433.show_register_settings();         //shows current CC1101 register values
  // init CC1101 RF-module and get My_address from EEPROM

  cc868.begin(4, 5, // gpio pins (dont use serial!)
    0x3, //set ISM Band 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz
    0x7,//set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb; 7 = OOK_raw
    0x0,//set channel
    3);//addr
  cc868.show_main_settings();             //shows setting debug messages to UART
  cc868.show_register_settings();         //shows current CC1101 register values

  ArduinoOTA.setHostname ( "ESPCC" ) ;                  // Set the hostname
  ArduinoOTA.begin() ;                               // Allow update over the air

  // cc1101.receive();                        //set to RECEIVE mode
}

// the loop function runs over and over again forever
void loop() {
  if (current_modulation){
    while (!current_modulation->loop(cc433));
    current_modulation = NULL;
  }
  ArduinoOTA.handle() ;                                 // Check for OTA
}