#pragma once
#include <Arduino.h>
#include <Wire.h>  // For LiquidCrystal
#include <LiquidCrystal.h>
#include <Adafruit_ST7735.h>

#ifndef DEBUG_PRINTLN
  #ifdef DEBUG
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINT(x) Serial.print(x)
  #else
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINT(x)
  #endif
#endif

void copystr(char *dest, const char *source, int len) {
    strncpy(dest, source, len);
    dest[len] = '\0';  // In case source did not fully copy, it is necessary to add termination character
}

void copystr(char *dest, const __FlashStringHelper *source, int len) {
    strncpy_P(dest, (const char*) source, len);
    dest[len] = '\0';  // In case source did not fully copy, it is necessary to add termination character
}

void padString(char* str, int len) {  
    // Returns a string with spaces added to reach len (useful for LCD screen, not so much for TFT screen)
    // Note: This has no checks about the memory size allocated to str, so it had better be long enough! 
    int slen = strlen(str); // Get length of current string up to but not including null character
    memset(str+slen, ' ', len-slen);  // Set spaces for the rest of the string up to len
    str[len] = '\0'; // Rewrite the null character at the end to make it a valid string
}

class ScreenOut {
    private:
        Adafruit_ST7735 *tft;

    public:
        ScreenOut(Adafruit_ST7735 *tft) : tft(tft) {
        }

        void begin() {
            tft->initR(INITR_144GREENTAB);
            tft->setRotation(3);
            tft->fillScreen(ST77XX_BLACK);
        }

        void writeScreen(char* mainMessage, byte switchPos, byte motorPos) {
            char displayText[50];  // TODO: Something wrong with using 17 chars, looks like the \0 is being overwritten or soemthing
            byte maxLen = 30;
            copystr(displayText, mainMessage, maxLen);
            padString(displayText, maxLen);  

            tft->setCursor(0,0);
            tft->print(displayText);
            
            snprintf(displayText, maxLen, "%d  %d", switchPos, motorPos);
            padString(displayText, maxLen);
            tft->setCursor(0,20);
            tft->print(displayText);
        }
        // void writeMode1() {
        //     // Main messages in top, resistance in ohms and position in volts of motor in bottom
        //     char displayText[17];
        //     copystr(displayText, mainMessage, 16);
        //     padString(displayText, 16);
            
        //     screen->setCursor(0,0);
        //     screen->print(displayText);

        //     char sfloat[5];
        //     dtostrf(motorVolts, 4, 2, sfloat);
        //     snprintf(displayText, 16, "Ohms: %d, Volts: %s", switchResistance, sfloat);
        //     padString(displayText, 16);
            
        //     screen->setCursor(0,1);
        //     screen->print(displayText);
        // }
};

class OtherOutputs {
    private: 
        char mainMessage[33];  // Main message text (sized to fit 32 characters plus \0 termination)
        int switchPos = -1; 
        int switchResistance = -1;
        int motorPos = -1;
        float motorVolts = -1;
        int displayMode = 0;  // So screen can display different information based on selected mode
        char motorMessage[33]; // Message from Motor
        ScreenOut screenOut;

        void writeDisplay() {
            screenOut.writeScreen(mainMessage, switchPos, motorPos);
        }        

        void writeFakePinOuts() {
            // Set pin outs to trick the Car into thinking it's in a certain state
            // TODO: Set pin outs (probably using pwm analog out with a lowpass filter?)
        }

    public:
        OtherOutputs(Adafruit_ST7735 *tft) : screenOut(tft) {
        } 

        void begin() {
            screenOut.begin();
            // TODO: Set pin outs for whatever I end up using to trick car
        }

        void writeOutputs() {
            // Output signals to trick car into thinking it's in correct state
            // And display screen
            writeFakePinOuts();
            writeDisplay();
        }

        void setMainMessage (const char *message) {
            copystr(mainMessage, message, 16);
            writeOutputs();
        }

        void setMainMessage (const __FlashStringHelper *message) {
            // const char *buffer = (const char PROGMEM *)message;
            copystr(mainMessage, message, 16);
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
            copystr(motorMessage, message, 16);
            writeOutputs();
        }

        void setMotorMessage(const __FlashStringHelper *message) {
            copystr(mainMessage, message, 16);
            writeOutputs();
        }
};