/*
    Pulse Generator

    Simple flip-flop pulse generator with adjustable minimal, maximal and
    current working frequency. Adjusting is perfomed by rotary encoder,
    shown on display.

    The circuit:
    * Arduino Nano
    * Rotary encoder KY-040
    * I2C OLED display 0,96" 128x64 (SSD-1306)

    Created 2019-05-02 By Yoda-81
    Modified 2019-05-03 By Yoda-81

    https://github.com/Yoda-81

*/

#include <Arduino.h>

#define ENCODER_CLK 5
#define ENCODER_DT 4
#define ENCODER_SW 3

int pinClkRetain;
int pinDtRetain;
int pinSwRetain;

void setup() {
    Serial.begin(9600);
    Serial.println("Rotary encoder");
    
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    
    pinClkRetain = digitalRead(ENCODER_CLK);
    pinDtRetain = digitalRead(ENCODER_DT);
    pinSwRetain = digitalRead(ENCODER_SW);
}

void loop() {
    int pinClk = digitalRead(ENCODER_CLK);
    int pinDt = digitalRead(ENCODER_DT);

    if (pinClk != pinClkRetain || pinDt != pinDtRetain) {
        if (pinDt == pinClkRetain && pinClk != pinDtRetain) {
            Serial.println("<-");
        } else if (pinClk == pinDtRetain && pinDt != pinClkRetain) {
            Serial.println("->");
        }
        pinClkRetain = pinClk;
        pinDtRetain = pinDt;
    }

    /*
    if (pinClkRetain != pinClk) {
        pinClkRetain = pinClk;
        if (pinClk == HIGH) {
            int pinDt = digitalRead(ENCODER_DT);
            if (pinClk == pinDt) {
                Serial.println("->");
            } else {
                Serial.println("<-");
            }
        }
    }*/

    int pinSw = digitalRead(ENCODER_SW);
    if (pinSwRetain != pinSw) {
        pinSwRetain = pinSw;
        if (pinSw == HIGH) {
            Serial.println("[ ]");
        } else {
            Serial.println("[X]");
        }
    }
}
