#ifndef UNIX_HOST_DUINO
#include "esp_arduino_spi.h"
#define PIN_SPI_MISO (12)

namespace cc1100 {

    void ESPArduinoSPI::begin(uint8_t pin) {
        ss_pin = pin;
        SPI.setHwCs(false);
        SPI.begin();
        pinMode(ss_pin, OUTPUT);
    }
    void ESPArduinoSPI::beginTransaction() {
        SPI.beginTransaction(settings);
        digitalWrite(ss_pin, LOW);
        ESP.wdtDisable();
    }
    void ESPArduinoSPI::endTransaction() {
        digitalWrite(ss_pin, HIGH);
        SPI.endTransaction();
        ESP.wdtEnable(1000);
    }
    void ESPArduinoSPI::write_strobe(uint8_t instr)
    {
        beginTransaction();
        SPI.write(instr);
        endTransaction();

    }

    void ESPArduinoSPI::reset_sequence(void)
    {
        digitalWrite(ss_pin, LOW);
        delayMicroseconds(10);
        digitalWrite(ss_pin, HIGH);
        delayMicroseconds(40);
        digitalWrite(ss_pin, LOW);
        SPI.beginTransaction(settings);
        SPI.write(SRES);
        SPI.endTransaction();
        pinMode(PIN_SPI_MISO, INPUT);
        while(digitalRead(PIN_SPI_MISO)==1) delayMicroseconds(1);
        pinMode(PIN_SPI_MISO, SPECIAL);
        digitalWrite(ss_pin, HIGH);
    }

    uint8_t ESPArduinoSPI::read_register(uint8_t instr)
    {
        beginTransaction();
        SPI.write(instr | READ_SINGLE_BYTE);
        uint8_t res = SPI.transfer(0xFF); // written data is ignored
        endTransaction();
        return res;
    }
    void ESPArduinoSPI::read_burst(uint8_t instr, uint8_t *pArr, uint8_t length)
    {
        beginTransaction();
        SPI.write(instr | READ_BURST);

        for(uint8_t i=0; i<length; i++)
        {
            pArr[i] = SPI.transfer(0xFF); // written data is ignored
        }
        endTransaction();
    }
    void ESPArduinoSPI::write_register(uint8_t instr, uint8_t value)
    {
        beginTransaction();
        SPI.write(instr | WRITE_SINGLE_BYTE);
        SPI.write(value);
        endTransaction();
    }
    void ESPArduinoSPI::write_burst(uint8_t instr, uint8_t *pArr, uint8_t length)
    {
        beginTransaction();
        SPI.write(instr | WRITE_BURST);

        for(uint8_t i=0; i<length ;i++)
        {
            SPI.write(pArr[i]);
        }
        endTransaction();
    }
}
#endif