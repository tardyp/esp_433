/*
  ESP8266 Blink by Simon Peter
  Blink the blue LED on the ESP-01 module
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/
#include <Arduino.h>
#include <AUnit.h>
#include "cc1100_arduino.h"
#include "esp_arduino_spi.h"
#include "modulations.h"

cc1100::ESPArduinoSPI spi433;
cc1100::ESPArduinoSPI spi868;
cc1100::CC1100 cc433(spi433);
cc1100::CC1100 cc868(spi868);

test(ook_pwm_modulation) {
  cc1100::Modulation *m = cc1100::Modulation::make("n=name,m=OOK_PWM,s=426,l=852,r=16000,g=3000,t=0,y=6,rows=2");
  assertNotEqual((void*)m, (void*)0);
}
void setup() {
}

// the loop function runs over and over again forever
void loop() {
    aunit::TestRunner::run();


}