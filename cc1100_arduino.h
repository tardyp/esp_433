#ifndef cc1100_H
#define cc1100_H
#include <Arduino.h>

namespace cc1100 {

const int TRUE=1;
const int FALSE=0;

//|=====================[ setting EEPROM addresses]=============================
const int EEPROM_ADDRESS_CC1100_FREQUENCY= 0x1F4;  //ISM band
const int EEPROM_ADDRESS_CC1100_MODE=      0x1F5;  //modulation mode
const int EEPROM_ADDRESS_CC1100_MY_ADDR=   0x1F6;  //receiver address
const int EEPROM_ADDRESS_CC1100_CHANNEL=   0x1F7;  //channel number


/*----------------------[CC1100 - misc]---------------------------------------*/
const int CRYSTAL_FREQUENCY=         26000000;
const int CFG_REGISTER=              0x2F;  //47 registers
const int FIFOBUFFER=                0x42;  //size of Fifo Buffer
const int RSSI_OFFSET_868MHZ=        0x4E;  //dec = 74
const int TX_RETRIES_MAX=            0x05;  //tx_retries_max
const int ACK_TIMEOUT=                200;  //ACK timeout in ms
const int CC1100_COMPARE_REGISTER=   0x00;  //register compare 0=no compare 1=compare
const int BROADCAST_ADDRESS=         0x00;  //broadcast address
const int CC1100_FREQ_315MHZ=        0x01;
const int CC1100_FREQ_434MHZ=        0x02;
const int CC1100_FREQ_868MHZ=        0x03;
const int CC1100_FREQ_915MHZ=        0x04;
//const int CC1100_FREQ_2430MHZ=       0x05;
const float CC1100_TEMP_ADC_MV=        3.225; //3.3V/1023 . mV pro digit
const float CC1100_TEMP_CELS_CO=       2.47;  //Temperature coefficient 2.47mV per Grad Celsius

/*---------------------------[CC1100 - R/W offsets]---------------------------*/
const int WRITE_SINGLE_BYTE=   0x00;
const int WRITE_BURST=         0x40;
const int READ_SINGLE_BYTE=    0x80;
const int READ_BURST=          0xC0;
/*---------------------------[END R/W offsets]--------------------------------*/

/*------------------------[CC1100 - FIFO commands]----------------------------*/
const int TXFIFO_BURST=        0x7F;    //write burst only
const int TXFIFO_SINGLE_BYTE=  0x3F;    //write single only
const int RXFIFO_BURST=        0xFF;    //read burst only
const int RXFIFO_SINGLE_BYTE=  0xBF;    //read single only
const int PATABLE_BURST=       0x7E;    //power control read/write
const int PATABLE_SINGLE_BYTE= 0xFE;    //power control read/write
/*---------------------------[END FIFO commands]------------------------------*/

/*----------------------[CC1100 - config register]----------------------------*/
const int IOCFG2=   0x00;         // GDO2 output pin configuration
const int IOCFG1=   0x01;         // GDO1 output pin configuration
const int IOCFG0=   0x02;         // GDO0 output pin configuration
const int FIFOTHR=  0x03;         // RX FIFO and TX FIFO thresholds
const int SYNC1=    0x04;         // Sync word, high byte
const int SYNC0=    0x05;         // Sync word, low byte
const int PKTLEN=   0x06;         // Packet length
const int PKTCTRL1= 0x07;         // Packet automation control
const int PKTCTRL0= 0x08;         // Packet automation control
const int ADDR=     0x09;         // Device address
const int CHANNR=   0x0A;         // Channel number
const int FSCTRL1=  0x0B;         // Frequency synthesizer control
const int FSCTRL0=  0x0C;         // Frequency synthesizer control
const int FREQ2=    0x0D;         // Frequency control word, high byte
const int FREQ1=    0x0E;         // Frequency control word, middle byte
const int FREQ0=    0x0F;         // Frequency control word, low byte
const int MDMCFG4=  0x10;         // Modem configuration
const int MDMCFG3=  0x11;         // Modem configuration
const int MDMCFG2=  0x12;         // Modem configuration
const int MDMCFG1=  0x13;         // Modem configuration
const int MDMCFG0=  0x14;         // Modem configuration
const int DEVIATN=  0x15;         // Modem deviation setting
const int MCSM2=    0x16;         // Main Radio Cntrl State Machine config
const int MCSM1=    0x17;         // Main Radio Cntrl State Machine config
const int MCSM0=    0x18;         // Main Radio Cntrl State Machine config
const int FOCCFG=   0x19;         // Frequency Offset Compensation config
const int BSCFG=    0x1A;         // Bit Synchronization configuration
const int AGCCTRL2= 0x1B;         // AGC control
const int AGCCTRL1= 0x1C;         // AGC control
const int AGCCTRL0= 0x1D;         // AGC control
const int WOREVT1=  0x1E;         // High byte Event 0 timeout
const int WOREVT0=  0x1F;         // Low byte Event 0 timeout
const int WORCTRL=  0x20;         // Wake On Radio control
const int FREND1=   0x21;         // Front end RX configuration
const int FREND0=   0x22;         // Front end TX configuration
const int FSCAL3=   0x23;         // Frequency synthesizer calibration
const int FSCAL2=   0x24;         // Frequency synthesizer calibration
const int FSCAL1=   0x25;         // Frequency synthesizer calibration
const int FSCAL0=   0x26;         // Frequency synthesizer calibration
const int RCCTRL1=  0x27;         // RC oscillator configuration
const int RCCTRL0=  0x28;         // RC oscillator configuration
const int FSTEST=   0x29;         // Frequency synthesizer cal control
const int PTEST=    0x2A;         // Production test
const int AGCTEST=  0x2B;         // AGC test
const int TEST2=    0x2C;         // Various test settings
const int TEST1=    0x2D;         // Various test settings
const int TEST0=    0x2E;         // Various test settings
/*-------------------------[END config register]------------------------------*/

/*------------------------[CC1100-command strobes]----------------------------*/
const int SRES=     0x30;         // Reset chip
const int SFSTXON=  0x31;         // Enable/calibrate freq synthesizer
const int SXOFF=    0x32;         // Turn off crystal oscillator.
const int SCAL=     0x33;         // Calibrate freq synthesizer & disable
const int SRX=      0x34;         // Enable RX.
const int STX=      0x35;         // Enable TX.
const int SIDLE=    0x36;         // Exit RX / TX
const int SAFC=     0x37;         // AFC adjustment of freq synthesizer
const int SWOR=     0x38;         // Start automatic RX polling sequence
const int SPWD=     0x39;         // Enter pwr down mode when CSn goes hi
const int SFRX=     0x3A;         // Flush the RX FIFO buffer.
const int SFTX=     0x3B;         // Flush the TX FIFO buffer.
const int SWORRST=  0x3C;         // Reset real time clock.
const int SNOP=     0x3D;         // No operation.
/*-------------------------[END command strobes]------------------------------*/

/*----------------------[CC1100 - status register]----------------------------*/
const int PARTNUM=        0xF0;   // Part number
const int VERSION=        0xF1;   // Current version number
const int FREQEST=        0xF2;   // Frequency offset estimate
const int LQI=            0xF3;   // Demodulator estimate for link quality
const int RSSI=           0xF4;   // Received signal strength indication
const int MARCSTATE=      0xF5;   // Control state machine state
const int WORTIME1=       0xF6;   // High byte of WOR timer
const int WORTIME0=       0xF7;   // Low byte of WOR timer
const int PKTSTATUS=      0xF8;   // Current GDOx status and packet status
const int VCO_VC_DAC=     0xF9;   // Current setting from PLL cal module
const int TXBYTES=        0xFA;   // Underflow and # of bytes in TXFIFO
const int RXBYTES=        0xFB;   // Overflow and # of bytes in RXFIFO
const int RCCTRL1_STATUS= 0xFC;   //Last RC Oscillator Calibration Result
const int RCCTRL0_STATUS= 0xFD;   //Last RC Oscillator Calibration Result
//--------------------------[END status register]-------------------------------

class ISPI
{
    public:
        virtual void write_strobe(uint8_t instr) = 0;
        virtual void write_register(uint8_t instr, uint8_t value) = 0;
        virtual void write_burst(uint8_t instr, uint8_t *pArr, uint8_t length) = 0;
        virtual void read_burst(uint8_t instr, uint8_t *pArr, uint8_t length) = 0;
        virtual uint8_t read_register(uint8_t instr) = 0;
        virtual void reset_sequence() = 0;
};

class CC1100
{
    private:

        void spi_begin(void);
        void spi_end(void);
        uint8_t spi_putc(uint8_t data);
        ISPI &spi;
        int GDO0;
        int GDO2;
        uint8_t freq_select, mode_select, channel_select;
        uint8_t addr;

    public:
        CC1100(ISPI &spi): spi(spi) {};
        uint8_t debug_level;

        uint8_t set_debug_level(uint8_t set_debug_level);
        uint8_t get_debug_level(void);

        uint8_t begin(int gdo0, int gdo2, uint8_t freq_select, uint8_t mode_select, uint8_t channel_select, uint8_t addr);
        void end(void);

        void reset(void);
        void wakeup(void);
        void powerdown(void);

        void wor_enable(void);
        void wor_disable(void);
        void wor_reset(void);

        uint8_t sidle(void);
        uint8_t transmit(void);
        uint8_t receive(void);

        void show_register_settings(void);
        void show_main_settings(void);

        uint8_t packet_available();
        uint8_t wait_for_packet(uint16_t milliseconds);

        uint8_t get_payload(uint8_t rxbuffer[], uint8_t &pktlen_rx,uint8_t &my_addr,
                                      uint8_t &sender, int8_t &rssi_dbm, uint8_t &lqi);

        uint8_t tx_payload_burst(uint8_t my_addr, uint8_t rx_addr, uint8_t *txbuffer, uint8_t length);
        uint8_t rx_payload_burst(uint8_t rxbuffer[], uint8_t &pktlen);

        void rx_fifo_erase(uint8_t *rxbuffer);
        void tx_fifo_erase(uint8_t *txbuffer);

        uint8_t tx_fifo_bytes();
        void tx_fifo_fill(uint8_t *txbuffer, uint8_t length);
        void start_transmit();

        uint8_t tx_raw_transmit(uint8_t *txbuffer, uint8_t length);

        /* txbuffer payload starts at byte 3! [0] is len, [1] is srcaddr, [2] is dstaddr */
        uint8_t send_packet(uint8_t my_addr, uint8_t rx_addr, uint8_t *txbuffer, uint8_t pktlen, uint8_t tx_retries);
        void send_acknowledge(uint8_t my_addr, uint8_t tx_addr);

        uint8_t check_acknowledge(uint8_t *rxbuffer, uint8_t pktlen, uint8_t sender, uint8_t my_addr);

        int8_t rssi_convert(uint8_t Rssi);
        uint8_t check_crc(uint8_t lqi);
        uint8_t lqi_convert(uint8_t lqi);
        uint8_t get_temp(uint8_t *ptemp_Arr);

        void set_myaddr(uint8_t addr);
        void set_channel(uint8_t channel);
        void set_ISM(uint8_t ism_freq);
        void set_mode(uint8_t mode);
        void set_output_power_level(int8_t dbm);
        void set_patable(uint8_t *patable_arr);
        void set_fec(uint8_t cfg);
        void set_data_whitening(uint8_t cfg);
        void set_modulation_type(uint8_t cfg);
        void set_preamble_len(uint8_t cfg);
        void set_manchester_encoding(uint8_t cfg);
        void set_sync_mode(uint8_t cfg);
        void set_datarate(uint8_t mdmcfg4, uint8_t mdmcfg3, uint8_t deviant);

        void uart_puthex_nibble(const unsigned char b);
        void uart_puthex_byte(const unsigned char  b);
        void uart_puti(const int val);

};
//=======================[CC1100 special functions]=============================
}
void DBG(char *fmt, ... );
#endif // H
