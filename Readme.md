# esp_433

Gereric 433 868 receiver and transmitter using CC1101 and ESP8266 / ESP32

This cc1101 driver aimed to use raw infinite fifo mode of CC1101 in order to receive and transmitnon standard modulations like the one found in cheap RF home modules

Goal is to use the same modulation definition as rtl_433 so that modulation can be reversed engineered with rtl_433 tool
and then easily decoded/transmitted using low power cc1101 module

https://triq.org/rtl_433/OPERATION.html#flex-decoder

heavy WIP

