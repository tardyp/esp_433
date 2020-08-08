/*
  ESP8266 Blink by Simon Peter
  Blink the blue LED on the ESP-01 module
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/
#ifdef UNIX_HOST_DUINO

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
  cc1100::Modulation *m = cc1100::Modulation::make("m=OOK_PWM,s=426,l=852,r=16000,g=3000,t=1,y=6,rows=2");
  assertNotEqual((void *)m, (void *)0);
  assertEqual((int)m->_short, 426);
  assertEqual((int)m->_long, 852);
  assertEqual((int)m->_reset, 16000);
  assertEqual((int)m->_gap, 3000);
  assertEqual((int)m->_tolerance, 1);
  assertEqual((int)m->_sync, 6);
  m = cc1100::Modulation::make("m=unknown_mod,s=426,l=852,r=16000,g=3000,t=1,y=6,rows=2");
  assertEqual((void *)m, (void *)0);
  m = cc1100::Modulation::make("s=426,l=852,r=16000,m=OOK_PWM,g=3000,t=1,y=6,rows=2");
  assertNotEqual((void *)m, (void *)0);
  m->start_send(cc433);

  int DRATE_E = spi433.read_register(0x10) & 0xf;
  int DRATE_M = spi433.read_register(0x11);
  /* we need to hack a bit the official calculation to git in 32bits */
  int drate = (26000000 >> 10) * ((256 + DRATE_M) << DRATE_E) / (1 << 18);
  int bit_duration = 1000000 / drate;
  assertEqual(bit_duration, 426); /* well, that is the same as _short! */
}
namespace cc1100
{
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
  uint8_t buffer[16];
  int written;
  cc1100::Modulation *m;
  m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=2,r=10,g=3,t=1,y=6,data=F");
  m->start_send(cc433);
  written = m->next_buffer(buffer, 16);
  assertEqual(written, 1);
  assertEqual(buffer[0], 0b10101010);

  m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=2,r=10,g=3,t=1,y=6,data=E");
  m->start_send(cc433);
  written = m->next_buffer(buffer, 16);
  assertEqual(written, 2);
  assertEqual(buffer[0], 0b10101011); /* last  bit it double 1 */
  assertEqual(buffer[1], 0b0);

  m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=2,r=10,g=3,t=1,y=6,data=EE");
  m->start_send(cc433);
  written = m->next_buffer(buffer, 16);
  assertEqual(written, 3);
  assertEqual(buffer[0], 0b10101011);
  assertEqual(buffer[1], 0b01010101);
  assertEqual(buffer[2], 0b10000000);

  m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=15,r=10,g=3,t=1,y=6,data=E");
  m->start_send(cc433);
  written = m->next_buffer(buffer, 16);
  assertEqual(written, 3);
  assertEqual(buffer[0], 0b10101011);
  assertEqual(buffer[1], 0b11111111);
  assertEqual(buffer[2], 0b11111000);

  m = cc1100::Modulation::make("m=OOK_PWM,s=1,l=2,r=10,g=10,t=1,y=6,data=E:E");
  m->start_send(cc433);
  written = m->next_buffer(buffer, 16);
  assertEqual(written, 4);
  assertEqual(buffer[0], 0b10101011);
  assertEqual(buffer[1], 0b00000000);
  assertEqual(buffer[2], 0b00010101);
  assertEqual(buffer[3], 0b01100000);
}
test(ook_pwm_test_modulation_data)
{
  uint8_t buffer[15];
  int written = 15;
  cc1100::Modulation *m;
  m = cc1100::Modulation::make("m=OOK_TEST,s=1,l=2,r=10,g=10,t=1,y=6");
  m->start_send(cc433);
  while(written == 15) {
    written = m->next_buffer(buffer, 15);
  }
  assertEqual(written, 1);

}
namespace cc1100
{
  class FakeModulation : public Modulation
  {
  public:
    int to_send;
    inline FakeModulation(int _to_send)
    {
      to_send = _to_send;
      state = STARTING;
    }
    virtual void start_send(CC1100 &cc);
    virtual int next_buffer(uint8_t *buffer, int len);
    virtual void end_send(CC1100 &cc);
  };
  void FakeModulation::start_send(CC1100 &cc)
  {
  }
  int FakeModulation::next_buffer(uint8_t *buffer, int len)
  {
      printf("next_buffer %d %d\n", len, to_send);

      if (to_send < len) {
        len = to_send;
      }
      to_send -= len;
      return len;
  }
  void FakeModulation::end_send(CC1100 &cc)
  {
  }
} // namespace cc1100
test(modulation_loop)
{
  cc1100::FakeModulation m(300);
  spi433.write_register(cc1100::MARCSTATE, 1);
  assertEqual(m.loop(cc433), false);
  spi433.write_register(cc1100::TXBYTES, cc1100::FIFOBUFFER >> 1);
  assertEqual(m.loop(cc433), false);
  spi433.write_register(cc1100::TXBYTES, 20);
  for(int i; i< 500; i++)
    assertEqual(m.loop(cc433), false);
  /* simulate fifo empty */
  spi433.write_register(cc1100::TXBYTES, 0);
  spi433.write_register(cc1100::MARCSTATE, 1);
  assertEqual(m.loop(cc433), true);
}
void setup()
{
}

// the loop function runs over and over again forever
void loop()
{
  aunit::TestRunner::run();
}
#endif