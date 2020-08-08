#ifdef UNIX_HOST_DUINO
#include "esp_arduino_spi.h"
#define PIN_SPI_MISO (12)

namespace cc1100 {
    static uint8_t regs[256];
    void ESPArduinoSPI::begin(uint8_t pin) {
    }
    void ESPArduinoSPI::beginTransaction() {
    }
    void ESPArduinoSPI::endTransaction() {
    }
    void ESPArduinoSPI::write_strobe(uint8_t instr)
    {
    }

    void ESPArduinoSPI::reset_sequence(void)
    {
    }

    uint8_t ESPArduinoSPI::read_register(uint8_t instr)
    {
        return regs[instr];
    }
    void ESPArduinoSPI::read_burst(uint8_t instr, uint8_t *pArr, uint8_t length)
    {
    }
    void ESPArduinoSPI::write_register(uint8_t instr, uint8_t value)
    {
        regs[instr] = value;
    }
    void ESPArduinoSPI::write_burst(uint8_t instr, uint8_t *pArr, uint8_t length)
    {
    }
}
#endif