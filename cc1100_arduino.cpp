/*------------------------------------------------------------------------------
'                     CC1100 Arduino Library
'                     ----------------------
'
'
'
'
'
'  module contains helper code from other people. Thx for that
'-----------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdarg.h>
#include "cc1100_arduino.h"

#ifdef UNIX_HOST_DUINO
#define delayMicroseconds(x) do {} while(0)
#endif

/* https://playground.arduino.cc/Main/Printf/ */
void DBG(char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

namespace cc1100 {

#ifndef __AVR__
#define eeprom_read_block memcpy
#endif
//-------------------[global EEPROM default settings 868 Mhz]-------------------
extern uint8_t cc1100_GFSK_1_2_kb[];
extern uint8_t cc1100_GFSK_38_4_kb[];
extern uint8_t cc1100_GFSK_100_kb[];
extern uint8_t cc1100_MSK_250_kb[];

extern uint8_t cc1100_MSK_500_kb[];
extern uint8_t cc1100_OOK_4_8_kb[];
extern uint8_t cc1100_OOK_raw[];

//Patable index: -30  -20- -15  -10   0    5    7    10 dBm
extern uint8_t patable_power_315[];
extern uint8_t patable_power_433[];
extern uint8_t patable_power_868[];
extern uint8_t patable_power_915[];

void CC1100::reset(void)                  // reset defined in cc1100 datasheet
{
    spi.reset_sequence();
}

void CC1100::powerdown(void)
{
    sidle();
    spi.write_strobe(SPWD);               // CC1100 Power Down
}

void CC1100::wakeup(void)
{
    spi.reset_sequence();
    receive();                            // go to RX Mode
}

uint8_t CC1100::set_debug_level(uint8_t set_debug_level = 1)  //default ON
{
    debug_level = set_debug_level;        //set debug level of CC1101 outputs

    return debug_level;
}

uint8_t CC1100::get_debug_level(void)
{
    return debug_level;
}

uint8_t CC1100::begin(int gdo0, int gdo2, uint8_t freq_select, uint8_t mode_select, uint8_t channel_select, uint8_t addr)
{
    uint8_t partnum, version;
    GDO0 = gdo0;
    GDO2 = gdo2;
    this->freq_select = freq_select;
    this->mode_select = mode_select;
    this->channel_select = channel_select;
    this->addr = addr;

    pinMode(GDO0, INPUT);                 //setup AVR GPIO ports
    pinMode(GDO2, INPUT);
    set_debug_level();   //set debug level of CC1101 outputs

    if(debug_level > 0){
        Serial.println(F("Init CC1100..."));
    }

    reset();                              //CC1100 init reset
    spi.write_strobe(SFTX);delayMicroseconds(100);//flush the TX_fifo content
    spi.write_strobe(SFRX);delayMicroseconds(100);//flush the RX_fifo content

    partnum = spi.read_register(PARTNUM); //reads CC1100 partnumber
    version = spi.read_register(VERSION); //reads CC1100 version number

    //checks if valid Chip ID is found. Usualy 0x03 or 0x14. if not -> abort
    if(version == 0x00 || version == 0xFF){
            if(debug_level > 0){
                Serial.print(F("no CC11xx found!"));
                Serial.println();
            }

            return FALSE;
        }

    if(debug_level > 0){
        Serial.print(F("Partnumber:"));
        uart_puthex_byte(partnum);
        Serial.println();

        Serial.print(F("Version:"));
        uart_puthex_byte(version);
        Serial.println();
    }

    //if no valid settings are available, CC100 Powerdown and SPI disable
    if( addr == 0xFF ||
        freq_select == 0xFF ||
        mode_select == 0xFF ||
        channel_select == 0xFF)
        {
            if(debug_level > 0){
                Serial.print(F("no EEPROM settings..."));
                Serial.println();
            }
            end();                        //CC1100 Powerdown and disable SPI bus

            return FALSE;
        }

    //set modulation mode
    set_mode(mode_select);

    //set ISM band
    set_ISM(freq_select);

    //set channel
    set_channel(channel_select);

    //set output power amplifier
    set_output_power_level(0);            //set PA to 0dBm as default

    //set my receiver address
    set_myaddr(addr);                  //Addr from EEPROM to global variable

    if(debug_level > 0){
        Serial.println(F("...done"));
    }

    return TRUE;
}

void CC1100::end(void)
{
    powerdown();                          //power down CC1100
}

void CC1100::show_register_settings(void)
{
    if(debug_level > 0){
        uint8_t config_reg_verify[CFG_REGISTER],Patable_verify[CFG_REGISTER];

        spi.read_burst(READ_BURST,config_reg_verify,CFG_REGISTER);  //reads all 47 config register
        spi.read_burst(PATABLE_BURST,Patable_verify,8);             //reads output power settings

        //show_main_settings();
        Serial.println(F("Cfg_reg:"));

        for(uint8_t i = 0 ; i < CFG_REGISTER; i++)  //showes rx_buffer for debug
            {
                uart_puthex_byte(config_reg_verify[i]);Serial.print(F(" "));
                if(i==9 || i==19 || i==29 || i==39) //just for beautiful output style
                    {
                        Serial.println();
                    }
            }
            Serial.println();
            Serial.println(F("PaTable:"));

            for(uint8_t i = 0 ; i < 8; i++)         //showes rx_buffer for debug
                {
                    uart_puthex_byte(Patable_verify[i]);Serial.print(F(" "));
                }
        Serial.println();
    }
}

void CC1100::show_main_settings(void)
{
    if(debug_level > 0){
        Serial.print(F("Mode:"));
        Serial.print(mode_select);
        Serial.println();

        Serial.print(F("Frequency:"));
        Serial.print(freq_select);
        Serial.println();

        Serial.print(F("Channel:"));
        Serial.print(channel_select);
        Serial.println();

        Serial.print(F("My_Addr:"));
        Serial.print(addr);
        Serial.println();
    }
}

uint8_t CC1100::sidle(void)
{
    uint8_t marcstate;

    spi.write_strobe(SIDLE);              //sets to idle first. must be in

    marcstate = 0xFF;                     //set unknown/dummy state value

    while(marcstate != 0x01)              //0x01 = sidle
    {
        marcstate = (spi.read_register(MARCSTATE) & 0x1F); //read out state of cc1100 to be sure in RX
        //uart_puthex_byte(marcstate);
    }
    //Serial.println();
    delayMicroseconds(100);
    return TRUE;
}

uint8_t CC1100::transmit(void)
{
    uint8_t marcstate;

    sidle();                              //sets to idle first.
    spi.write_strobe(STX);                //sends the data over air

    do {
        marcstate = (spi.read_register(MARCSTATE) & 0x1F); //read out state of cc1100 to be sure in IDLE and TX is finished
        //uart_puthex_byte(marcstate);
    } while(marcstate != 0x01);              //0x01 = ILDE after sending data
    //Serial.println();
    delayMicroseconds(100);
    return TRUE;
}

uint8_t CC1100::receive(void)
{
    uint8_t marcstate;

    sidle();                              //sets to idle first.
    spi.write_strobe(SRX);                //writes receive strobe (receive mode)

    marcstate = 0xFF;                     //set unknown/dummy state value

    while(marcstate != 0x0D)              //0x0D = RX
    {
        marcstate = (spi.read_register(MARCSTATE) & 0x1F); //read out state of cc1100 to be sure in RX
        //uart_puthex_byte(marcstate);
    }
    //Serial.println();
    delayMicroseconds(100);
    return TRUE;
}


//------------[enables WOR Mode  EVENT0 ~1890ms; rx_timeout ~235ms]--------------------
void CC1100::wor_enable()
{
/*
    EVENT1 = WORCTRL[6:4] -> Datasheet page 88
    EVENT0 = (750/Xtal)*(WOREVT1<<8+WOREVT0)*2^(5*WOR_RES) = (750/26Meg)*65407*2^(5*0) = 1.89s

                        (WOR_RES=0;RX_TIME=0)               -> Datasheet page 80
i.E RX_TIMEOUT = EVENT0*       (3.6038)      *26/26Meg = 235.8ms
                        (WOR_RES=0;RX_TIME=1)               -> Datasheet page 80
i.E.RX_TIMEOUT = EVENT0*       (1.8029)      *26/26Meg = 117.9ms
*/
    sidle();

    spi.write_register(MCSM0, 0x18);    //FS Autocalibration
    spi.write_register(MCSM2, 0x01);    //MCSM2.RX_TIME = 1b

    // configure EVENT0 time
    spi.write_register(WOREVT1, 0xFF);  //High byte Event0 timeout
    spi.write_register(WOREVT0, 0x7F);  //Low byte Event0 timeout

    // configure EVENT1 time
    spi.write_register(WORCTRL, 0x78);  //WOR_RES=0b; tEVENT1=0111b=48d -> 48*(750/26MHz)= 1.385ms

    spi.write_strobe(SFRX);             //flush RX buffer
    spi.write_strobe(SWORRST);          //resets the WOR timer to the programmed Event 1
    spi.write_strobe(SWOR);             //put the radio in WOR mode when CSn is released

    delayMicroseconds(100);
}

void CC1100::wor_disable()
{
    sidle();                            //exit WOR Mode
    spi.write_register(MCSM2, 0x07);    //stay in RX. No RX timeout
}

void CC1100::wor_reset()
{
    sidle();                            //go to IDLE
    spi.write_register(MCSM2, 0x01);    //MCSM2.RX_TIME = 1b
    spi.write_strobe(SFRX);             //flush RX buffer
    spi.write_strobe(SWORRST);          //resets the WOR timer to the programmed Event 1
    spi.write_strobe(SWOR);             //put the radio in WOR mode when CSn is released

    delayMicroseconds(100);
}

uint8_t CC1100::tx_payload_burst(uint8_t my_addr, uint8_t rx_addr,
                              uint8_t *txbuffer, uint8_t length)
{
    txbuffer[0] = length-1;
    txbuffer[1] = rx_addr;
    txbuffer[2] = my_addr;

    spi.write_burst(TXFIFO_BURST,txbuffer,length); //writes TX_Buffer +1 because of pktlen must be also transfered

    if(debug_level > 0){
        Serial.print(F("TX_FIFO:"));
        for(uint8_t i = 0 ; i < length; i++)       //TX_fifo debug out
        {
            uart_puthex_byte(txbuffer[i]);
        }
        Serial.println();
  }
  return TRUE;
}

uint8_t CC1100::rx_payload_burst(uint8_t rxbuffer[], uint8_t &pktlen)
{
    uint8_t bytes_in_RXFIFO = 0;
    uint8_t res = 0;

    bytes_in_RXFIFO = spi.read_register(RXBYTES);              //reads the number of bytes in RXFIFO

    if((bytes_in_RXFIFO & 0x7F) && !(bytes_in_RXFIFO & 0x80))  //if bytes in buffer and no RX Overflow
    {
        spi.read_burst(RXFIFO_BURST, rxbuffer, bytes_in_RXFIFO);
        pktlen = rxbuffer[0];
        res = TRUE;
    }
    else
    {
        if(debug_level > 0){
            Serial.print(F("no bytes in RX buffer or RX Overflow!: "));Serial.println(bytes_in_RXFIFO);
        }
        sidle();                                                  //set to IDLE
        spi.write_strobe(SFRX);delayMicroseconds(100);            //flush RX Buffer
        receive();                                                //set to receive mode
        res = FALSE;
    }

    return res;
}

uint8_t CC1100::tx_fifo_bytes(){
    return spi.read_register(TXBYTES)&0x7f; //reads the number of bytes in TXFIFO
}

void CC1100::tx_fifo_fill(uint8_t *txbuffer, uint8_t length){
    spi.write_burst(TXFIFO_BURST, txbuffer, length);
}
void CC1100::start_transmit(void){
    spi.write_strobe(STX);                //start sending the data over air
}
/* transmit large buffer via infinite packet feature */
uint8_t CC1100::tx_raw_transmit(uint8_t *txbuffer, uint8_t length)
{
    uint8_t remain, marcstate, to_send;
    uint8_t started = 0;
    sidle();                              //sets to idle first.

    while(length > 0) {
        to_send = (FIFOBUFFER>>1);
        if (to_send > length)
            to_send = length;

        tx_fifo_fill(txbuffer, to_send);
        txbuffer += to_send;
        length -= to_send;
        if (!started){
            start_transmit();
        }
        // watch the FIFO until it is half empty
        do {
            remain = tx_fifo_bytes(); //reads the number of bytes in TXFIFO
        } while (remain > (FIFOBUFFER>>1) );
    }
    do {
        remain = tx_fifo_bytes(); //reads the number of bytes in TXFIFO
    } while (remain > 0);

    sidle();                              //sets to idle when everything's done.
    return TRUE;
}
uint8_t CC1100::send_packet(uint8_t my_addr, uint8_t rx_addr, uint8_t *txbuffer,
                            uint8_t pktlen,  uint8_t tx_retries)
{
    uint8_t pktlen_ack;                                         //default package len for ACK
    uint8_t rxbuffer[FIFOBUFFER];
    uint8_t tx_retries_count = 0;
    uint8_t from_sender;
    uint16_t ackWaitCounter = 0;

    if(pktlen > (FIFOBUFFER - 1))                               //FIFO overflow check
    {
        printf("ERROR: package size overflow\r\n");
        return FALSE;
    }

    do                                                          //sent package out with retries
    {
        tx_payload_burst(my_addr, rx_addr, txbuffer, pktlen);   //loads the data in cc1100 buffer
        transmit();                                             //sents data over air
        receive();                                              //receive mode

        if(rx_addr == BROADCAST_ADDRESS){                       //no wait acknowledge if sent to broadcast address or tx_retries = 0
            return TRUE;                                        //successful sent to BROADCAST_ADDRESS
        }

        while (ackWaitCounter < ACK_TIMEOUT )                   //wait for an acknowledge
        {
            if (packet_available() == TRUE)                     //if RF package received check package acknowge
            {
                from_sender = rx_addr;                          //the original message sender address
                rx_fifo_erase(rxbuffer);                        //erase RX software buffer
                rx_payload_burst(rxbuffer, pktlen_ack);         //reads package in buffer
                check_acknowledge(rxbuffer, pktlen_ack, from_sender, my_addr); //check if received message is an acknowledge from client
                return TRUE;                                    //package successfully sent
            }
            else{
                ackWaitCounter++;                               //increment ACK wait counter
                delay(1);                                       //delay to give receiver time
            }
        }

        ackWaitCounter = 0;                                     //resets the ACK_Timeout
        tx_retries_count++;                                     //increase tx retry counter

        if(debug_level > 0){                                    //debug output messages
            Serial.print(F(" #:"));
            uart_puthex_byte(tx_retries_count-1);
            Serial.println();
      }
    }while(tx_retries_count <= tx_retries);                     //while count of retries is reaches

    return FALSE;                                               //sent failed. too many retries
}

void CC1100::send_acknowledge(uint8_t my_addr, uint8_t tx_addr)
{
    uint8_t pktlen = 0x06;                                      //complete Pktlen for ACK packet
    uint8_t tx_buffer[0x06];                                    //tx buffer array init

    tx_buffer[3] = 'A'; tx_buffer[4] = 'c'; tx_buffer[5] = 'k'; //fill buffer with ACK Payload

    tx_payload_burst(my_addr, tx_addr, tx_buffer, pktlen);      //load payload to CC1100
    transmit();                                                 //sent package over the air
    receive();                                                  //set CC1100 in receive mode

    if(debug_level > 0){                                        //debut output
        Serial.println(F("Ack_sent!"));
    }
}
//-------------------------------[end]------------------------------------------
//----------------------[check if Packet is received]---------------------------
uint8_t CC1100::packet_available()
{
    if(digitalRead(GDO2) == TRUE)                           //if RF package received
    {
        if(spi.read_register(IOCFG2) == 0x06)               //if sync word detect mode is used
        {
            while(digitalRead(GDO2) == TRUE){               //wait till sync word is fully received
                Serial.println(F("!"));
            }
        }

        if(debug_level > 0){
            //Serial.println("Pkt->:");
        }
        return TRUE;
    }
    return FALSE;
}

uint8_t CC1100::get_payload(uint8_t rxbuffer[], uint8_t &pktlen, uint8_t &my_addr,
                               uint8_t &sender, int8_t &rssi_dbm, uint8_t &lqi)
{
    uint8_t crc;

    rx_fifo_erase(rxbuffer);                               //delete rx_fifo bufffer

    if(rx_payload_burst(rxbuffer, pktlen) == FALSE)        //read package in buffer
    {
        rx_fifo_erase(rxbuffer);                           //delete rx_fifo bufffer
        return FALSE;                                    //exit
    }
    else
    {
        my_addr = rxbuffer[1];                             //set receiver address to my_addr
        sender = rxbuffer[2];

        if(check_acknowledge(rxbuffer, pktlen, sender, my_addr) == TRUE) //acknowlage received?
        {
            rx_fifo_erase(rxbuffer);                       //delete rx_fifo bufffer
            return FALSE;                                //Ack received -> finished
        }
        else                                               //real data, and sent acknowladge
        {
            rssi_dbm = rssi_convert(rxbuffer[pktlen + 1]); //converts receiver strength to dBm
            lqi = lqi_convert(rxbuffer[pktlen + 2]);       //get rf quialtiy indicator
            crc = check_crc(lqi);                          //get packet CRC

            if(debug_level > 0){                           //debug output messages
                if(rxbuffer[1] == BROADCAST_ADDRESS)       //if my receiver address is BROADCAST_ADDRESS
                {
                    Serial.println(F("BROADCAST message"));
                }

                Serial.print(F("RX_FIFO:"));
                for(uint8_t i = 0 ; i < pktlen + 1; i++)   //showes rx_buffer for debug
                {
                    uart_puthex_byte(rxbuffer[i]);
                }
                Serial.print(" |");
                uart_puthex_byte(rxbuffer[pktlen+1]);
                uart_puthex_byte(rxbuffer[pktlen+2]);
                Serial.print("|");
                Serial.println();

                Serial.print(F("RSSI:"));uart_puti(rssi_dbm);Serial.print(F(" "));
                Serial.print(F("LQI:"));uart_puthex_byte(lqi);Serial.print(F(" "));
                Serial.print(F("CRC:"));uart_puthex_byte(crc);
                Serial.println();
            }

            my_addr = rxbuffer[1];                         //set receiver address to my_addr
            sender = rxbuffer[2];                          //set from_sender address

            if(my_addr != BROADCAST_ADDRESS)               //send only ack if no BROADCAST_ADDRESS
            {
                send_acknowledge(my_addr, sender);           //sending acknowledge to sender!
            }

            return TRUE;
        }
        return FALSE;
    }
}

uint8_t CC1100::check_acknowledge(uint8_t *rxbuffer, uint8_t pktlen, uint8_t sender, uint8_t my_addr)
{
    int8_t rssi_dbm;
    uint8_t crc, lqi;

    if((pktlen == 0x05 && \
       (rxbuffer[1] == my_addr || rxbuffer[1] == BROADCAST_ADDRESS)) && \
        rxbuffer[2] == sender && \
        rxbuffer[3] == 'A' && rxbuffer[4] == 'c' && rxbuffer[5] == 'k') //acknowledge received!
        {
            if(rxbuffer[1] == BROADCAST_ADDRESS){                           //if receiver address BROADCAST_ADDRESS skip acknowledge
                if(debug_level > 0){
                    Serial.println(F("BROADCAST ACK"));
                }
                return FALSE;
            }
            rssi_dbm = rssi_convert(rxbuffer[pktlen + 1]);
            lqi = lqi_convert(rxbuffer[pktlen + 2]);
            crc = check_crc(lqi);

            if(debug_level > 0){
                //Serial.println();
                Serial.print(F("ACK! "));
                Serial.print(F("RSSI:"));uart_puti(rssi_dbm);Serial.print(F(" "));
                Serial.print(F("LQI:"));uart_puthex_byte(lqi);Serial.print(F(" "));
                Serial.print(F("CRC:"));uart_puthex_byte(crc);
                Serial.println();
            }
            return TRUE;
        }
    return FALSE;
}

uint8_t CC1100::wait_for_packet(uint16_t milliseconds)
{
    for(uint16_t i = 0; i < milliseconds; i++)
        {
            delay(1);                 //delay till system has data available
            if (packet_available())
            {
                return TRUE;
            }
        }
    //Serial.println(F("no packet received!"));
    return FALSE;
}

void CC1100::tx_fifo_erase(uint8_t *txbuffer)
{
    memset(txbuffer, 0, sizeof(FIFOBUFFER));  //erased the TX_fifo array content to "0"
}

void CC1100::rx_fifo_erase(uint8_t *rxbuffer)
{
    memset(rxbuffer, 0, sizeof(FIFOBUFFER)); //erased the RX_fifo array content to "0"
}

void CC1100::set_myaddr(uint8_t addr)
{
    spi.write_register(ADDR,addr);          //stores MyAddr in the CC1100
}

void CC1100::set_channel(uint8_t channel)
{
    spi.write_register(CHANNR,channel);   //stores the new channel # in the CC1100
}


//-[set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb]-
void CC1100::set_mode(uint8_t mode)
{
    uint8_t Cfg_reg[CFG_REGISTER];

    switch (mode)
    {
        case 0x01:
                    eeprom_read_block(Cfg_reg,cc1100_GFSK_1_2_kb,CFG_REGISTER);
                    break;
        case 0x02:
                    eeprom_read_block(Cfg_reg,cc1100_GFSK_38_4_kb,CFG_REGISTER);
                    break;
        case 0x03:
                    eeprom_read_block(Cfg_reg,cc1100_GFSK_100_kb,CFG_REGISTER);
                    break;
        case 0x04:
                    eeprom_read_block(Cfg_reg,cc1100_MSK_250_kb,CFG_REGISTER);
                    break;
        case 0x05:
                    eeprom_read_block(Cfg_reg,cc1100_MSK_500_kb,CFG_REGISTER);
                    break;
        case 0x06:
                    eeprom_read_block(Cfg_reg,cc1100_OOK_4_8_kb,CFG_REGISTER);
                    break;
        case 0x07:
                    eeprom_read_block(Cfg_reg,cc1100_OOK_raw,CFG_REGISTER);
                    break;
        default:
                    eeprom_read_block(Cfg_reg,cc1100_GFSK_38_4_kb,CFG_REGISTER);
                    mode = 0x02;
                    break;
    }

    spi.write_burst(WRITE_BURST,Cfg_reg,CFG_REGISTER);                            //writes all 47 config register
}

//---------[set ISM Band 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz]----------------
void CC1100::set_ISM(uint8_t ism_freq)
{
    uint8_t freq2, freq1, freq0;
    uint8_t Patable[8];

    switch (ism_freq)                                                       //loads the RF freq which is defined in cc1100_freq_select
    {
        case 0x01:                                                          //315MHz
                    freq2=0x0C;
                    freq1=0x1D;
                    freq0=0x89;
                    eeprom_read_block(Patable,patable_power_315,8);
                    break;
        case 0x02:                                                          //433.92MHz
                    freq2=0x10;
                    freq1=0xB0;
                    freq0=0x71;
                    eeprom_read_block(Patable,patable_power_433,8);
                    break;
        case 0x03:                                                          //868.3MHz
                    freq2=0x21;
                    freq1=0x65;
                    freq0=0x6A;
                    eeprom_read_block(Patable,patable_power_868,8);
                    break;
        case 0x04:                                                          //915MHz
                    freq2=0x23;
                    freq1=0x31;
                    freq0=0x3B;
                    eeprom_read_block(Patable,patable_power_915,8);
                    break;
        default:                                                             //default is 868.3MHz
                    freq2=0x21;
                    freq1=0x65;
                    freq0=0x6A;
                    eeprom_read_block(Patable,patable_power_868,8);          //sets up output power ramp register
                    ism_freq = 0x03;
                    break;
    }

    spi.write_register(FREQ2,freq2);                                         //stores the new freq setting for defined ISM band
    spi.write_register(FREQ1,freq1);
    spi.write_register(FREQ0,freq0);

    spi.write_burst(PATABLE_BURST,Patable,8);                                //writes output power settings to cc1100
}

void CC1100::set_patable(uint8_t *patable_arr)
{
    spi.write_burst(PATABLE_BURST,patable_arr,8);   //writes output power settings to cc1100    "104us"
}

void CC1100::set_output_power_level(int8_t dBm)
{
    uint8_t pa = 0xC0;

    if      (dBm <= -30) pa = 0x00;
    else if (dBm <= -20) pa = 0x01;
    else if (dBm <= -15) pa = 0x02;
    else if (dBm <= -10) pa = 0x03;
    else if (dBm <= 0)   pa = 0x04;
    else if (dBm <= 5)   pa = 0x05;
    else if (dBm <= 7)   pa = 0x06;
    else if (dBm <= 10)  pa = 0x07;

    spi.write_register(FREND0,pa);
}

void CC1100::set_modulation_type(uint8_t cfg)
{
    uint8_t data;
    data = spi.read_register(MDMCFG2);
    data = (data & 0x8F) | (((cfg) << 4) & 0x70);
    spi.write_register(MDMCFG2, data);
    //printf("MDMCFG2: 0x%02X\n", data);
}

void CC1100::set_preamble_len(uint8_t cfg)
{
    uint8_t data;
    data = spi.read_register(MDMCFG1);
    data = (data & 0x8F) | (((cfg) << 4) & 0x70);
    spi.write_register(MDMCFG1, data);
    //printf("MDMCFG2: 0x%02X\n", data);
}

//-------------------[set modem datarate and deviant]--------------------------
void CC1100::set_datarate(uint8_t mdmcfg4, uint8_t mdmcfg3, uint8_t deviant)
{
    spi.write_register(MDMCFG4, mdmcfg4);
    spi.write_register(MDMCFG3, mdmcfg3);
    spi.write_register(DEVIATN, deviant);
}

//----------------------[set sync mode no sync=0;]-----------------------------
void CC1100::set_sync_mode(uint8_t cfg)
{
    uint8_t data;
    data = spi.read_register(MDMCFG2);
    data = (data & 0xF8) | (cfg & 0x07);
    spi.write_register(MDMCFG2, data);
    //printf("MDMCFG2: 0x%02X\n", data);
}

//---------------[set FEC ON=TRUE; OFF=FALSE]----------------------------------
void CC1100::set_fec(uint8_t cfg)
{
    uint8_t data;
    data = spi.read_register(MDMCFG1);
    data = (data & 0x7F) | (((cfg) << 7) & 0x80);
    spi.write_register(MDMCFG1, data);
}

//---------------[set data_whitening ON=TRUE; OFF=FALSE]------------------------
void CC1100::set_data_whitening(uint8_t cfg)
{
    uint8_t data;
    data = spi.read_register(PKTCTRL0);
    data = (data & 0xBF) | (((cfg) << 6) & 0x40);
    spi.write_register(PKTCTRL0, data);
}

//------------[set manchester encoding ON=TRUE; OFF=FALSE]---------------------
void CC1100::set_manchester_encoding(uint8_t cfg)
{
    uint8_t data;
    data = spi.read_register(MDMCFG2);
    data = (data & 0xF7) | (((cfg) << 3) & 0x08);
    spi.write_register(MDMCFG2, data);
}

//--------------------------[rssi_convert]--------------------------------------
int8_t CC1100::rssi_convert(uint8_t Rssi_hex)
{
    int8_t rssi_dbm;
    int16_t Rssi_dec;

    Rssi_dec = Rssi_hex;        //convert unsigned to signed

    if(Rssi_dec >= 128){
        rssi_dbm=((Rssi_dec-256)/2)-RSSI_OFFSET_868MHZ;
    }
    else{
        if(Rssi_dec<128){
            rssi_dbm=((Rssi_dec)/2)-RSSI_OFFSET_868MHZ;
        }
    }
    return rssi_dbm;
}

//----------------------------[lqi convert]-------------------------------------
uint8_t CC1100::lqi_convert(uint8_t lqi)
{
    return (lqi & 0x7F);
}

//----------------------------[check crc]---------------------------------------
uint8_t CC1100::check_crc(uint8_t lqi)
{
    return (lqi & 0x80);
}

//----------------------------[get temp]----------------------------------------
uint8_t CC1100::get_temp(uint8_t *ptemp_Arr)
{
    const uint8_t num_samples = 8;
    uint16_t adc_result = 0;
    uint32_t temperature = 0;

    sidle();                              //sets CC1100 into IDLE
    spi.write_register(PTEST,0xBF);       //enable temp sensor
    delay(50);                            //wait a bit

    for(uint8_t i=0;i<num_samples;i++)    //sampling analog temperature value
    {
        adc_result += analogRead(GDO0);
        delay(1);
    }
    adc_result = adc_result / num_samples;
    //Serial.println(adc_result);

    temperature = (adc_result * CC1100_TEMP_ADC_MV) / CC1100_TEMP_CELS_CO;

    ptemp_Arr[0] = temperature / 10;      //cut last digit
    ptemp_Arr[1] = temperature % 10;      //isolate last digit

    if(debug_level > 0){
        Serial.print(F("Temp:"));Serial.print(ptemp_Arr[0]);Serial.print(F("."));Serial.println(ptemp_Arr[1]);
    }

    spi.write_register(PTEST,0x7F);       //writes 0x7F back to PTest (app. note)

    receive();
    return (*ptemp_Arr);
}

//|=========================== UART helper ====================================|

/*******************************************************************************
Function: uart_puthex_nibble()
Purpose:  transmit lower nibble as ASCII-hex to UART
Input:    byte value
Returns:  none
This functions has been added by Martin Thomas <eversmith@heizung-thomas.de>
Don't blame P. Fleury if it doesn't work ;-)
*******************************************************************************/
void CC1100::uart_puthex_nibble(const unsigned char b)
{
    unsigned char  c = b & 0x0f;
    if (c>9) c += 'A'-10;
    else c += '0';
    Serial.write(c);                  //uart_putc replaced to Serial.write
} /* uart_puthex_nibble */

/*******************************************************************************
Function: uart_puthex_byte()
Purpose:  transmit upper and lower nibble as ASCII-hex to UART
Input:    byte value
Returns:  none
This functions has been added by Martin Thomas <eversmith@heizung-thomas.de>
Don't blame P. Fleury if it doesn't work ;-)
*******************************************************************************/
void CC1100::uart_puthex_byte(const unsigned char b)
{
    uart_puthex_nibble(b>>4);
    uart_puthex_nibble(b);
} /* uart_puthex_byte */

/*******************************************************************************
Function: uart_puti()
Purpose:  transmit integer as ASCII to UART
Input:    integer value
Returns:  none
This functions has been added by Martin Thomas <eversmith@heizung-thomas.de>
Don't blame P. Fleury if it doesn't work ;-)
*******************************************************************************/
void CC1100::uart_puti(const int val)
{
    char buffer[sizeof(int)*8+1];
#ifdef UNIX_HOST_DUINO
    printf("%d", val);
#else
    Serial.printf("%d", val);
#endif
}/* uart_puti */

//|================================= END ======================================|
}