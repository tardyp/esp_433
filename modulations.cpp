#include <Arduino.h>

#include "modulations.h"

namespace cc1100 {
    Modulation *Modulation::make(String definition_string)
    {
        Modulation **m;
        for(m = modulations; m[0]; m++){
            if (m[0]->name == "OOK_PWM"){
                return m[0];
            }
        }
        return NULL;
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
