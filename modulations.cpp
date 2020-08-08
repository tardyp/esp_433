#include <Arduino.h>

#include "modulations.h"
const uint32_t F_OSC = 26000000;

namespace cc1100
{
    int log2(int x)
    {
        /* find the highest bit of a value (integer version of log2) */
        int r = -1;
        while (x)
        {
            x >>= 1;
            r++;
        }
        return r;
    }
    int hcf(int a, int b)
    {
        /* finds the highest common factor */
        int r;
        while (b != 0)
        {
            r = a % b;
            a = b;
            b = r;
        }
        return a;
    }
    uint8_t hex2i(uint8_t v)
    {
        if ((v >= '0') && (v <= '9'))
            return v - '0';
        if ((v >= 'A') && (v <= 'F'))
            return v - 'A' + 0xA;
        if ((v >= 'a') && (v <= 'f'))
            return v - 'a' + 0xA;
        return 0;
    }
    Modulation *Modulation::make(String definition_string)
    {
        Modulation *m = NULL;
        int key_pos = 0, value_pos;
        String value, key;
        while (key_pos < definition_string.length())
        {
            value_pos = definition_string.indexOf('=', key_pos);
            if (value_pos < 0)
                return NULL;
            key = definition_string.substring(key_pos, value_pos);
            key_pos = definition_string.indexOf(',', value_pos);
            if (key_pos < 0)
                key_pos = definition_string.length();
            value = definition_string.substring(value_pos + 1, key_pos);
            key_pos += 1;
            if (key == "m")
            {
                if (m != NULL)
                {
                    continue;
                }
                Modulation **m_it;
                for (m_it = modulations; m_it[0]; m_it++)
                {
                    if (m_it[0]->name == value)
                    {
                        printf("found %s\r\n", value.c_str());
                        m = m_it[0];
                        m->fill_default_values();
                        break;
                    }
                }
                if (m == NULL)
                {
                    Serial.print(String("Modulation: ") + value + " not supported\r\n");
                    return NULL;
                }
                /* restart scan from start once we got our modulation type */
                key_pos = 0;
            }
            else
            {
                if (m == NULL)
                {
                    /* continue scan until we get our modulation type */
                    continue;
                }
                m->set_param(key, value);
            }
        }
        return m;
    }
    void Modulation::fill_default_values()
    {
        this->_short = 100;
        this->_long = 200;
        this->_sync = 1;
        this->_reset = 2000;
        this->_gap = 400;
        this->_tolerance = 1;
        this->_data = "";
    }
    void Modulation::set_param(String k, String v)
    {
        if (k == "short" || k == "s")
            this->_short = v.toInt();
        else if (k == "long" || k == "l")
            this->_long = v.toInt();
        else if (k == "sync" || k == "y")
            this->_sync = v.toInt();
        else if (k == "reset" || k == "r")
            this->_reset = v.toInt();
        else if (k == "gap" || k == "g")
            this->_gap = v.toInt();
        else if (k == "tolerance" || k == "t")
            this->_tolerance = v.toInt();
        else if (k == "data")
            this->_data = v;
    }
#define BURST_SIZE (FIFOBUFFER >> 2)
    bool Modulation::loop(CC1100 &cc)
    {
        uint8_t remain, to_send;
        uint8_t buf[BURST_SIZE];
        if (state == STARTING)
        {
            cc.sidle(); //sets to idle first.
        }
        else
        {
            /* fifo management */
            remain = cc.tx_fifo_bytes(); //reads the number of bytes in TXFIFO
            if (state == SENDING)
            {
                if (remain > (FIFOBUFFER >> 1))
                    /* need to wait a bit before next fifo fill */
                    return false;
            }
            else if (state == FINISHING)
            {
                if (remain == 0)
                {
                    cc.sidle(); //sets to idle when everything's done.
                    return true;
                }
                return false;
            }
        }
        /* half fill the fifo */
        to_send = BURST_SIZE;
        to_send = next_buffer(buf, to_send);
        cc.tx_fifo_fill(buf, to_send);
        if (state == STARTING)
        {
            cc.start_transmit(); //start sending the data over air
            state = SENDING;
        }
        if (to_send < BURST_SIZE)
        {
            state = FINISHING;
        }
        return false;
    }

    class OOK_PWM : public Modulation
    {
    public:
        uint32_t short_bit;
        uint32_t long_bit;
        uint32_t gap_bit;
        uint32_t data_index;
        uint8_t data_bit;
        uint8_t bit_to_write;
        uint32_t bit_to_write_number;
        uint8_t last_data_bit;

        inline OOK_PWM(String name)
        {
            this->name = name;
        };
        virtual void start_send(CC1100 &cc);
        virtual int next_buffer(uint8_t *buffer, int len);
        virtual void end_send(CC1100 &cc);
        virtual bool compute_next_bit(void);
    };

    void OOK_PWM::start_send(CC1100 &cc)
    {
        /* calculate required baudrate as the hcf of _short and long*/
        if (_tolerance == 0)
            _tolerance = 1;
        uint32_t time_per_bit = hcf(_short, _long) / _tolerance;
        short_bit = _short / time_per_bit;
        long_bit = _long / time_per_bit;
        gap_bit = _gap / time_per_bit;
        uint32_t rate = 1000000 / time_per_bit;

        /* magical formula as per cc1101 datasheet page 35 */
        uint8_t DRATE_E = log2((rate << 12) / (F_OSC >> 8));
        /* we need to hack a bit the calculation to fit in 32bit */
        /* probably more optim can be found with more thought */
        uint32_t DRATE_M = ((rate << (18 - DRATE_E)) / (F_OSC >> 10)) - 256;
        if (DRATE_M > 255)
        {
            DRATE_M = 0;
            DRATE_E++;
        }
        cc.set_datarate(0x80 | DRATE_E, DRATE_M, 0);
        data_index = 0;
        data_bit = 0;
        bit_to_write = 0;
        bit_to_write_number = 0;
        last_data_bit = 0;
        state = STARTING;
    }
    bool OOK_PWM::compute_next_bit(void)
    {
        if (bit_to_write == 0)
        {
            /* data is encoded as hex nibbles */
            if (data_index >= _data.length())
                return false;
            uint8_t v = _data.charAt(data_index);
            if (v == ':')
            {
                bit_to_write = 0;
                bit_to_write_number = gap_bit;
                data_index += 1;
                return true;
            }
            v = hex2i(v);
            v = (v >> (3 - data_bit)) & 1;
            last_data_bit = v;
            bit_to_write = 1;
            bit_to_write_number = v ? short_bit : long_bit;
            data_bit += 1;
            if (data_bit >= 4)
            {
                data_bit = 0;
                data_index += 1;
            }
        }
        else
        {
            bit_to_write = 0;
            bit_to_write_number = last_data_bit ? long_bit : short_bit;
        }
        return true;
    }
    int OOK_PWM::next_buffer(uint8_t *buffer, int len)
    {
        int written = 0;
        int bit = 0;
        uint8_t cur_byte = 0;
        while (len)
        {
            /* first we write to buffer, we may have some remain from previous call */
            if (bit_to_write_number >= 8 && bit == 0)
            { /* we write a complete byte */
                *buffer = bit_to_write ? 0xff : 0;
                buffer++;
                len--;
                written++;
                bit_to_write_number -= 8;
            }
            else if (bit_to_write_number >= 8 - bit)
            { /* we finish a byte */

                /* if bit != 0 we assume *buffer is already initialized */
                if (bit_to_write)
                    *buffer |= (0xff >> bit);
                buffer++;
                len--;
                bit_to_write_number -= 8 - bit;
                bit = 0;
            }
            else if (bit_to_write_number > 0)
            { /* we write at begining or middle of the byte */
                /* initialize current byte to 0s if we just start this byte*/
                if (bit == 0)
                {
                    written++;
                    *buffer = 0;
                }
                /* CC1101 transmits MSB first */
                if (bit_to_write)
                {
                    uint8_t mask = 0xff << (8 - bit_to_write_number);
                    *buffer |= mask >> bit;
                }
                bit += bit_to_write_number;
                bit_to_write_number = 0;
            }
            if (bit_to_write_number == 0)
                if (!compute_next_bit())
                    break;
        }
        return written;
    }
    void OOK_PWM::end_send(CC1100 &cc)
    {
    }

    Modulation *modulations[] = {
        new OOK_PWM(String("OOK_PWM")),
        NULL};
} // namespace cc1100
