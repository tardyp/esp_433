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

test(ook_pwm_modulation)
{
  cc1100::Modulation *m = cc1100::Modulation::make("m=OOK_PWM,s=426,l=852,r=16000,g=3000,t=0,y=6,rows=2");
  assertNotEqual((void *)m, (void *)0);
  assertEqual((int)m->_short, 426);
  assertEqual((int)m->_long, 852);
  assertEqual((int)m->_reset, 16000);
  assertEqual((int)m->_gap, 3000);
  assertEqual((int)m->_tolerance, 0);
  assertEqual((int)m->_sync, 6);
  m = cc1100::Modulation::make("m=unknown_mod,s=426,l=852,r=16000,g=3000,t=0,y=6,rows=2");
  assertEqual((void *)m, (void *)0);
  m = cc1100::Modulation::make("s=426,l=852,r=16000,m=OOK_PWM,g=3000,t=0,y=6,rows=2");
  assertNotEqual((void *)m, (void *)0);
  m->start_send(cc433);

  int DRATE_E = spi433.read_register(0x10) & 0xf;
  int DRATE_M = spi433.read_register(0x11);
  /* we need to hack a bit the official calculation to git in 32bits */
  int drate = (26000000 >> 10) * ((256 + DRATE_M) << DRATE_E) / (1 << 18);
  int bit_duration = 1000000 / drate;
  assertEqual(bit_duration, 426); /* well, that is the same as _short! */
}
namespace cc1100 {
    uint8_t hex2i(uint8_t v);
}
test(hex2i)
{
  assertEqual((int)cc1100::hex2i('a'), 0xa);
  assertEqual((int)cc1100::hex2i('1'), 0x1);
  assertEqual((int)cc1100::hex2i('2'), 0x2);
  assertEqual((int)cc1100::hex2i('B'), 0xb);
}

test(ook_pwm_modulation_data)
{
	u_int8_t buffer[16];
	int written;
  cc1100::Modulation *m;
	m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=2,r=10,g=3,t=0,y=6,data=0");
  m->start_send(cc433);
  written = m->next_buffer(buffer, 16);
  assertEqual(written, 1);
  assertEqual(buffer[0], 0b10101010);

  m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=2,r=10,g=3,t=0,y=6,data=1");
  m->start_send(cc433);
  written = m->next_buffer(buffer, 16);
  assertEqual(written, 2);
  assertEqual(buffer[0], 0b10101011);
  assertEqual(buffer[1], 0b0);

 //TBD
  // m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=2,r=10,g=3,t=0,y=6,data=11");
  // m->start_send(cc433);
  // written = m->next_buffer(buffer, 16);
  // assertEqual(written, 3);
  // assertEqual(buffer[0], 0b10101011);
  // assertEqual(buffer[1], 0b01010101);
  // assertEqual(buffer[1], 0b10000000);

}
void setup()
{
}

// the loop function runs over and over again forever
void loop()
{
  aunit::TestRunner::run();
}