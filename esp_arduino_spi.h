#pragma once
#include <Arduino.h>
#include "cc1100_arduino.h"

#ifndef UNIX_HOST_DUINO
#include <SPI.h>
#endif

namespace cc1100 {

    class ESPArduinoSPI: public ISPI {
        private:
            void beginTransaction(void);
            void endTransaction(void);

        public:
            uint8_t ss_pin;
            void begin(uint8_t pin);
#ifndef UNIX_HOST_DUINO
            SPISettings settings = SPISettings(2000000, MSBFIRST, SPI_MODE0);
#endif
            virtual void write_strobe(uint8_t instr);
            virtual void write_register(uint8_t instr, uint8_t value);
            virtual void write_burst(uint8_t instr, uint8_t *pArr, uint8_t length);
            virtual void read_burst(uint8_t instr, uint8_t *pArr, uint8_t length);
            virtual uint8_t read_register(uint8_t instr);
            virtual void reset_sequence(void);
    };
}
