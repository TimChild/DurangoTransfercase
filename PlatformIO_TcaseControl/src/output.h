#pragma once
#include <Arduino.h>
#include <Wire.h>  // For LiquidCrystal
#include <LiquidCrystal.h>
#include <Adafruit_ST7735.h>

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

String makeStringFixedLen(String str, int len) {
    // Returns a string with a spaces added to reach len
        str = str.substring(0, len); // Make sure only max len chars
        for(int i = int(str.length()); i < len; i++) {
            str += ' ';  
        }
    return str;
}

void padString(char* str, int len) {
    // Returns a string with spaces added to reach len
    // Note: This has no checks about the memory size allocated to str, so it had better be long enough! 
    int slen = strlen(str); // Get length of current string up to but not including null character
    memset(str+slen, ' ', len-slen-1);  // Set spaces for the rest of the string up to len
    str[len] = '\0'; // Rewrite the null character at the end to make it a valid string
}

class OtherOutputs {
    private: 
        char mainMessage[33];  // Main message text (sized to fit 32 characters plus \0 termination)
        int switchPos = -1; 
        int switchResistance = -1;
        int motorPos = -1;
        float motorVolts = -1;
        int displayMode = 0;  // So screen can display different information based on selected mode
        char motorMessage[33]; // Message from Motor
        // LiquidCrystal screen;
        Adafruit_ST7735 *screen;


        void writeDisplay() {
            switch (displayMode) {
                case 0:
                    writeMode0();
                    break;
                case 1:
                    writeMode1();
                    break;
                default:
                    writeMode0();
                    break;
            }
        }        

        void writeMode0() {
            // Main messages in top, selection as text for switch and motor in bottom
            delay(100);
            char displayText[17];
            strncpy(displayText, mainMessage, 16);
            padString(displayText, 16);

            screen->setCursor(0,0);
            screen->print(displayText);
            
            // snprintf(displayText, 16, "%d  %d", switchPos, motorPos);
            // padString(displayText, 16);
            // screen->setCursor(0,1);
            // screen->print(displayText);
            delay(50);
        }

        void writeMode1() {
            // Main messages in top, resistance in ohms and position in volts of motor in bottom
            char displayText[17];
            strncpy(displayText, mainMessage, 16);
            padString(displayText, 16);
            
            screen->setCursor(0,0);
            screen->print(displayText);

            char sfloat[5];
            dtostrf(motorVolts, 4, 2, sfloat);
            snprintf(displayText, 16, "Ohms: %d, Volts: %s", switchResistance, sfloat);
            padString(displayText, 16);
            
            screen->setCursor(0,1);
            screen->print(displayText);
        }

        void writeFakePinOuts() {
            // Set pin outs to trick the Car into thinking it's in a certain state
            // TODO: Set pin outs (probably using pwm analog out with a lowpass filter?)
        }

    public:
        // OtherOutputs() : screen(0,0,0,0,0,0) {
        // } // TODO: Almost definitely wrong and needs fixing
        OtherOutputs() {

        }

        OtherOutputs(Adafruit_ST7735 *tft) : screen(tft) {
            screen->initR(INITR_144GREENTAB);
            screen->setRotation(3);
            screen->fillScreen(ST77XX_BLACK);
        } 

        void setTFT(Adafruit_ST7735 *tft) {
            screen = tft;
            screen->initR(INITR_144GREENTAB);
            screen->setRotation(3);
            screen->fillScreen(ST77XX_BLACK);
        } 
        // void setLcd(LiquidCrystal lcd) {
        //     screen = lcd;
        //     screen.begin(16,2);
        //     screen.clear();
        // }

        void writeOutputs() {
            // Output signals to trick car into thinking it's in correct state
            // And display screen
            writeFakePinOuts();
            writeDisplay();
        }

        void setBrightness () {
            // Set brightness of screen (might not be supported)
            // TODO: Use this to control the contrast of the screen or maybe whether the screen is displayed or not (or LED on off?)
        }

        void setMainMessage (const char *message) {
            strncpy(mainMessage, message, 16);
            writeOutputs();
        }

        void setMainMessage (const __FlashStringHelper *message) {
            // const char *buffer = (const char PROGMEM *)message;
            strncpy_P(mainMessage, (const char*)message, 16);
            writeOutputs();
        }

        void setSwitchPos(int pos) {
            if (0 <= pos && pos <= 3) {
                switchPos = pos;
            }
            writeOutputs();
        }

        void setSwitchResistance(int resistance) {
            switchResistance = resistance;
            writeOutputs();
        }

        void setMotorPos(int pos) {
            if (0 <= pos && pos <= 3) {
                motorPos = pos;
            }
            writeOutputs();
        }
        
        void setMotorVolts(float volts) {
            motorVolts = volts;
            writeOutputs();
        }

        void setMotorMessage(const char *message) {
            strncpy(motorMessage, message, 16);
            writeOutputs();
        }

        void setMotorMessage(const __FlashStringHelper *message) {
            strncpy_P(motorMessage, (const char*)message, 16);
            writeOutputs();
        }
};