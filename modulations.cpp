#include <Arduino.h>

#include "modulations.h"

namespace cc1100 {
    Modulation *Modulation::make(String definition_string)
    {
        Modulation *m=NULL;
        int key_pos=0, value_pos;
        String value, key;
        while(key_pos < definition_string.length())
        {
            value_pos = definition_string.indexOf('=', key_pos);
            if (value_pos < 0)
                return NULL;
            key = definition_string.substring(key_pos, value_pos);
            key_pos = definition_string.indexOf(',', value_pos);
            if (key_pos < 0)
                key_pos = definition_string.length();
            value = definition_string.substring(value_pos+1, key_pos);
            key_pos +=1;
            if (key == "m"){
                if (m != NULL) {
                    continue;
                }
                Modulation **m_it;
                for(m_it = modulations; m_it[0]; m_it++){
                    if (m_it[0]->name == value){
                        printf("found %s\n", value.c_str());
                        m = m_it[0];
                        break;
                    }
                }
                if(m==NULL){
                    Serial.print(String("Modulation: ") + value + " not supported\n");
                    return NULL;
                }
                /* restart scan from start once we got our modulation type */
                key_pos = 0;
            } else {
                if(m==NULL){
                    /* continue scan until we get our modulation type */
                    continue;
                }
                m->set_param(key, value);
            }
        }
        return m;
    }

    class OOK_PWM: public Modulation {
        public:
        inline OOK_PWM(String name){
            this->name = name;
        };
        virtual void set_param(String k, String v);
        virtual void start_send(CC1100 &cc);
        virtual int next_buffer(u_int8_t *buffer, int len);
        virtual void end_send(CC1100 &cc);
    };

    void OOK_PWM::set_param(String k, String v){
        Serial.print(k + " " + v + "\n");
    }
    void OOK_PWM::start_send(CC1100 &cc){

    }
    int OOK_PWM::next_buffer(u_int8_t *buffer, int len){

    }
    void OOK_PWM::end_send(CC1100 &cc){

    }

    Modulation *modulations[]={
        new OOK_PWM(String("OOK_PWM")),
        NULL
    };
}
