#pragma once
#include <Arduino.h>
#include <Wire.h>  // For LiquidCrystal
#include <LiquidCrystal.h>
#include <Adafruit_ST7735.h>
#include "Images.h"

// #define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif


// const int SCREEN_WIDTH = 128;
// const int SCREEN_HEIGHT = 128;
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 240;
#define TOP_MARGIN 5
#define RIGHT_MARGIN 5
#define BOTTOM_MARGIN 5
#define LEFT_MARGIN 20
const byte SF = 2;  // Overall Scale Factor for display (i.e. 1 for 128x128px, 2 for 240x240px to make things look similar size)
const int maxChars = (SCREEN_WIDTH-RIGHT_MARGIN-LEFT_MARGIN)/6/SF;  // Max no. characters per row on screen

const uint16_t PINK = 0xF811;
const uint16_t BLUE_GREY = 0x3B9C;


bool isValid(int pos) {
    if (pos >= 0 && pos <= 3) {
        return true;
    } 
    return false;
}

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

void posToStr(char* text, int pos) {
    switch (pos) {
        case 0:
            sprintf(text, "4HI");
            break;
        case 1:
            sprintf(text, "AWD");
            break;
        case 2:
            sprintf(text, "N");
            break;
        case 3:
            sprintf(text, "4LO");
            break;
        default: 
            sprintf(text, "N/A");
            break;
    }
}


class ScreenOut {
    private:
        // Adafruit_ST7735 *tft;
        Adafruit_ST7789 *tft;
        uint16_t bgColor = ST7735_BLACK;
        uint16_t textColor = PINK;
        uint16_t boxColor = BLUE_GREY;
        byte currentLayout = 0;
        
        // Stores for displayed data so can check if things have changed
        char currentMainText[maxChars*4+1];
        int currentSwitchPos;
        int currentSwitchOhms;
        int currentMotorPos;
        float currentMotorVolts;
        bool currentMotorPosValid;

        void resetStored() {
            sprintf(currentMainText, " ");
            currentSwitchPos = 5;
            currentSwitchOhms = 0;
            currentMotorPos = 5;
            currentMotorVolts = -1.0;
            currentMotorPosValid = true;
        }

        void writeBlock(const char* text, const byte cursorPosX, const byte cursorPosY, const byte fontSize, const byte width, const byte rows) {
            tft->fillRect(cursorPosX, cursorPosY, width, fontSize*8*rows, bgColor); 
            byte textLen = strlen(text);
            byte charPerRow = maxChars-2;

            int i = 0;
            byte row = 0;
            char buffer[maxChars];
            while (i<textLen && row<rows) {
                copystr(buffer, text+i, charPerRow);
                writeText(buffer, cursorPosX, cursorPosY+8*fontSize*row, fontSize);
                row+=1;
                i+=charPerRow;
            }
        }

        void writeText(const char* text, const byte cursorPosX, const byte cursorPosY, const byte fontSize) {
            tft->setCursor(cursorPosX, cursorPosY);
            tft->setTextColor(textColor);
            tft->setTextSize(fontSize);
            tft->print(text);
        }

        void strikeThrough(const byte startX, const byte startY, const byte width, const byte fontsize) {
            byte endX = startX + width;
            tft->drawLine(startX, startY, endX-1, startY+8*fontsize-1, textColor);
            tft->drawLine(startX, startY+8*fontsize-1, endX-1, startY, textColor);
        }

        void initNormalLayout() {
            // Default screen layout

            tft->fillScreen(bgColor);
            tft->setTextSize(1*SF, 2*SF);
            tft->setTextColor(textColor);
            tft->setTextWrap(false);

            tft->drawRoundRect(LEFT_MARGIN, TOP_MARGIN, SCREEN_WIDTH/2-LEFT_MARGIN, 70*SF-TOP_MARGIN, 4*SF, boxColor);  // For Switch
            tft->setCursor(LEFT_MARGIN+4*SF, TOP_MARGIN+2*SF);
            tft->print(F("Switch"));
            tft->drawFastHLine(LEFT_MARGIN+4*SF, TOP_MARGIN+17*SF, SCREEN_WIDTH/2-LEFT_MARGIN - 8*SF, boxColor);

            tft->drawRoundRect(SCREEN_WIDTH/2, TOP_MARGIN+0*SF, SCREEN_WIDTH/2-RIGHT_MARGIN, 70*SF-TOP_MARGIN, 4*SF, boxColor);  // For Motor
            tft->setCursor(SCREEN_WIDTH/2+4*SF, TOP_MARGIN+2*SF);
            tft->print(F("Motor"));
            tft->drawFastHLine(SCREEN_WIDTH/2+4*SF, 17*SF+TOP_MARGIN, SCREEN_WIDTH/2 - 8*SF - RIGHT_MARGIN, boxColor);

            tft->drawRoundRect(LEFT_MARGIN+0, 70*SF+TOP_MARGIN, SCREEN_WIDTH-LEFT_MARGIN-RIGHT_MARGIN, SCREEN_HEIGHT-70*SF-BOTTOM_MARGIN-TOP_MARGIN, 4*SF, boxColor);  // For extra text

            currentLayout = 1;
        }

        void drawCat() {
            tft->fillScreen(bgColor);
            uint16_t colors [] = {0xF800, 0xFC00, 0xFFE0, 0x07E0, 0x001F, 0xF81F};
            long i = random(6);
            // tft->drawBitmap(0, 0, cat, 128, 128, colors[i]);
            tft->drawBitmap(0, 0, cat, 240, 240, colors[i]);
            // tft->drawBitmap(0, 0, cat, 128, 128, PINK);


        }


    public:
        // ScreenOut(Adafruit_ST7735 *tft) : tft(tft) {
        ScreenOut(Adafruit_ST7789 *tft) : tft(tft) {
        }


        void begin() {
            // tft->initR(INITR_144GREENTAB);
            tft->init(240, 240);
            // tft->setRotation(3);
            tft->setRotation(2);
            tft->fillScreen(ST77XX_BLACK);
            drawCat();
        }

        void writeNormalValues(const char* mainText, const int switchPos, const int switchOhms, const int motorPos, const float motorVolts, bool motorPosValid) {
            char buffer[maxChars+1];

            // Fill normal layout with values
            if (currentLayout != 1) {
                initNormalLayout();
                resetStored();
            }

            if (switchPos != currentSwitchPos) {
                DEBUG_PRINT(F("ScreenOut>WriteNormalValues: switchPos = ")); DEBUG_PRINT(switchPos); DEBUG_PRINTLN("");
                posToStr(buffer, switchPos);
                writeBlock(buffer, LEFT_MARGIN+4*SF, 25*SF+TOP_MARGIN, 2*SF, SCREEN_WIDTH/2-8*SF-LEFT_MARGIN, 1);
                currentSwitchPos = switchPos;
            }

            if (motorPos != currentMotorPos || motorPosValid != currentMotorPosValid) {
                posToStr(buffer, motorPos);
                writeBlock(buffer, SCREEN_WIDTH/2+4*SF, 25*SF+TOP_MARGIN, 2*SF, SCREEN_WIDTH/2-8*SF-RIGHT_MARGIN, 1);
                if (!motorPosValid) {
                    strikeThrough(SCREEN_WIDTH/2+4*SF, 25*SF+TOP_MARGIN, SCREEN_WIDTH/2-8*SF-RIGHT_MARGIN, 2*SF);
                }
                currentMotorPos = motorPos;
                currentMotorPosValid = motorPosValid;
            }

            if (abs((float)switchOhms/currentSwitchOhms - 1.0) > 0.015) {  // If changes by more than X%
                sprintf(buffer, "%d \351", switchOhms);
                writeBlock(buffer, LEFT_MARGIN+4*SF, TOP_MARGIN+50*SF, 1*SF, SCREEN_WIDTH/2-8*SF - LEFT_MARGIN, 1);
                currentSwitchOhms = switchOhms;
            }

            if (abs(motorVolts - currentMotorVolts) > 0.02) {
                if (motorVolts < 10 && motorVolts > -10) {
                    char temp[7];
                    dtostrf(motorVolts, 4, 3, temp);
                    sprintf(buffer, "%s V", temp);
                    writeBlock(buffer, SCREEN_WIDTH/2+4*SF, 50*SF+TOP_MARGIN, 1*SF, SCREEN_WIDTH/2-8*SF - RIGHT_MARGIN, 1);
                    currentMotorVolts = motorVolts;
                }
            }

            if (strcmp(mainText, currentMainText) != 0) {
                writeBlock(mainText, LEFT_MARGIN+4*SF, 75*SF+TOP_MARGIN, 1*SF, SCREEN_WIDTH-8*SF-LEFT_MARGIN-RIGHT_MARGIN, 4);
                snprintf(currentMainText, maxChars*4, mainText);
            }

        }

        void showCat() {
            drawCat();
            currentLayout = -1;
        }

};

class OtherOutputs {
    private: 
        char mainMessage[maxChars*4+1];  // Main message text (sized to fit 32 characters plus \0 termination)
        int switchPos = -1; 
        int switchResistance = -1;
        int motorPos = -1;
        bool motorPosValid = true;
        float motorVolts = -1;
        int displayMode = 0;  // So screen can display different information based on selected mode
        // byte fakeSwitchState = AWD;
        // byte fakeMotorState = AWD;
        // char motorMessage[33]; // Message from Motor
        ScreenOut screenOut;
        // uint8_t fakeSwitchPin;
        // uint8_t fakeMotorPin;

        void writeDisplay() {
            // screenOut.writeScreen(mainMessage, switchPos, motorPos);
            screenOut.writeNormalValues(mainMessage, switchPos, switchResistance, motorPos, motorVolts, motorPosValid);
        }        

        void writeFakePinOuts() {
            // Set pin outs to trick the Car into thinking it's in a certain state
            // if (isValid(switchPos)) {
            //     if (switchPos == AWD && fakeSwitchState != 0){
            //         digitalWrite(fakeSwitchPin, LOW);
            //         fakeSwitchState = 0;
            //     } else if (switchPos != AWD && fakeSwitchState != 1) {
            //         digitalWrite(fakeSwitchPin, HIGH);
            //         fakeSwitchState = 1;
            //     }
            // }
            // if (isValid(motorPos)) {
            //     if (motorPos == AWD && fakeMotorState != 0){
            //         digitalWrite(fakeMotorPin, LOW);
            //         fakeMotorState = 0;
            //     } else if (motorPos != AWD && fakeMotorState != 1) {
            //         digitalWrite(fakeMotorPin, HIGH);
            //         fakeMotorState = 1;
            //     }
            // }
        }

    public:
        // OtherOutputs(Adafruit_ST7735 *tft, uint8_t fakeSwitchPin, uint8_t fakeMotorPin) : screenOut(tft), fakeSwitchPin(fakeSwitchPin), fakeMotorPin(fakeMotorPin) {
        // OtherOutputs(Adafruit_ST7789 *tft, uint8_t fakeSwitchPin, uint8_t fakeMotorPin) : screenOut(tft), fakeSwitchPin(fakeSwitchPin), fakeMotorPin(fakeMotorPin) {
        // } 

        OtherOutputs(Adafruit_ST7789 *tft) : screenOut(tft) {
        } 

        void begin() {
            screenOut.begin();
            // TODO: Set pin outs for whatever I end up using to trick car
            // pinMode(fakeSwitchPin, OUTPUT);
            // pinMode(fakeMotorPin, OUTPUT);
            // digitalWrite(fakeSwitchPin, LOW);
            // digitalWrite(fakeMotorPin, LOW);
        }

        void writeOutputs() {
            // Output signals to trick car into thinking it's in correct state
            // And display screen
            // writeFakePinOuts();
            writeDisplay();
        }

        void setMainMessage (const char *message) {
            copystr(mainMessage, message, maxChars*4);
            writeOutputs();
        }

        void setMainMessage (const __FlashStringHelper *message) {
            // const char *buffer = (const char PROGMEM *)message;
            copystr(mainMessage, message, maxChars*4);
            writeOutputs();
        }
        
        void getMainMessage(char* str, int strlen) {
            copystr(str, mainMessage, strlen);
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

        void setMotorPos(int pos, int lastValid) {
            if (0 <= pos && pos <= 3) {
                motorPos = pos;
                motorPosValid = true;
            } else {
                motorPos = lastValid;
                motorPosValid = false;
            }
            writeOutputs();
        }
        
        void setMotorVolts(float volts) {
            motorVolts = volts;
            writeOutputs();
        }

        // void setMotorMessage(const char *message) {
        //     copystr(motorMessage, message, 16);
        //     writeOutputs();
        // }

        // void setMotorMessage(const __FlashStringHelper *message) {
        //     copystr(mainMessage, message, 16);
        //     writeOutputs();
        // }

        void showCat(int delay_ms) {
            screenOut.showCat();
            delay(delay_ms);
            writeOutputs();
        }
};