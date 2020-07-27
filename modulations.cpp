#include <Arduino.h>

#include "modulations.h"
const u_int32_t F_OSC = 26000000;

namespace cc1100
{
    int log2(int x)
    {
        /* find the highest bit */
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
        int r;
        while (b != 0)
        {
            r = a % b;
            a = b;
            b = r;
        }
        return a;
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
                        printf("found %s\n", value.c_str());
                        m = m_it[0];
                        break;
                    }
                }
                if (m == NULL)
                {
                    Serial.print(String("Modulation: ") + value + " not supported\n");
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
    }
    class OOK_PWM : public Modulation
    {
    public:
        inline OOK_PWM(String name)
        {
            this->name = name;
        };
        virtual void start_send(CC1100 &cc);
        virtual int next_buffer(u_int8_t *buffer, int len);
        virtual void end_send(CC1100 &cc);
    };

    void OOK_PWM::start_send(CC1100 &cc)
    {
        /* calculate required baudrate */
        u_int32_t rate = 1000000 / hcf(_short, _long);

        /* magical formula as per datasheet page 35 */
        u_int8_t DRATE_E = log2((rate << 20) / F_OSC);
        /* we need to hack a bit the calculation to fit in 32bit */
        /* probably more optim can be found with more thought */
        int DRATE_M = ((rate << (18 - DRATE_E)) / (F_OSC >> 10)) - 256;
        if (DRATE_M > 255)
        {
            DRATE_M = 0;
            DRATE_E++;
        }
        cc.set_datarate(0x80 | DRATE_E, DRATE_M, 0);
    }
    int OOK_PWM::next_buffer(u_int8_t *buffer, int len)
    {
    }
    void OOK_PWM::end_send(CC1100 &cc)
    {
    }

    Modulation *modulations[] = {
        new OOK_PWM(String("OOK_PWM")),
        NULL};
} // namespace cc1100
