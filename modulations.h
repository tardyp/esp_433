
#pragma once
#include <Arduino.h>
#include "cc1100_arduino.h"

namespace cc1100 {
    class Modulation {
        public:
        String name;
        int frequency;
        u_int32_t _short;
        u_int32_t _long;
        u_int32_t _sync;
        u_int32_t _reset;
        u_int32_t _gap;
        u_int32_t _tolerance;
        String _data; /* hex encoded data to send */
        /* set parameter from http or other protocol
            parameters shall be as specified in https://triq.org/rtl_433/OPERATION.html#flex-decoder
            so that we can reuse decoders from rtl_433
        */
        virtual void set_param(String k, String v);

        /* Following set of methods are programing the CC1101 for the requested protocol */
        virtual void start_send(CC1100 &cc) = 0;
        /* fill the buffer as needed return the number of bytes written */
        virtual int next_buffer(u_int8_t *buffer, int len) = 0;
        /* do any cleanup as needed */
        virtual void end_send(CC1100 &cc) = 0;
        static Modulation *make(String definition_string);
    };
    extern Modulation *modulations[];
}