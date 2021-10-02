#pragma once
#include <Arduino.h>
#include <Wire.h>  // For LiquidCrystal
#include <LiquidCrystal.h>
#include <Adafruit_ST7735.h>

#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif


const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 128;
const int maxChars = SCREEN_WIDTH/6;  // Max no. characters per row on screen

const uint16_t PINK = 0xF811;
const uint16_t BLUE_GREY = 0x3B9C;


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
        Adafruit_ST7735 *tft;
        uint16_t bgColor = ST7735_BLACK;
        uint16_t textColor = PINK;
        uint16_t boxColor = BLUE_GREY;
        byte currentLayout = 0;
        
        // Stores for displayed data so can check if things have changed
        char currentMainText[maxChars*4];
        int currentSwitchPos;
        int currentSwitchOhms;
        int currentMotorPos;
        float currentMotorVolts;

        void resetStored() {
            sprintf(currentMainText, " ");
            currentSwitchPos = 5;
            currentSwitchOhms = 0;
            currentMotorPos = 5;
            currentMotorVolts = -1.0;
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

        void initNormalLayout() {
            // Default screen layout
            tft->fillScreen(bgColor);
            tft->setTextSize(1, 2);
            tft->setTextColor(textColor);
            tft->setTextWrap(false);

            tft->drawRoundRect(0, 0, SCREEN_WIDTH/2, 70, 4, boxColor);  // For Switch
            tft->setCursor(4, 2);
            tft->print(F("Switch"));
            tft->drawFastHLine(4, 17, SCREEN_WIDTH/2 - 8, boxColor);

            tft->drawRoundRect(SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, 70, 4, boxColor);  // For Motor
            tft->setCursor(SCREEN_WIDTH/2+4, 2);
            tft->print(F("Motor"));
            tft->drawFastHLine(SCREEN_WIDTH/2+4, 17, SCREEN_WIDTH/2 - 8, boxColor);

            tft->drawRoundRect(0, 70, SCREEN_WIDTH, SCREEN_HEIGHT-70, 4, boxColor);  // For extra text

            currentLayout = 1;
        }

        void drawCat() {
            const byte r = SCREEN_HEIGHT/2-20;
            const byte cx = SCREEN_WIDTH/2;
            const byte cy = SCREEN_HEIGHT/2+0.1*r;
            const uint16_t color = PINK;

            tft->fillScreen(ST7735_BLACK);
            tft->drawCircle(cx, cy, r, color);
            drawCatSymParts(1, r, cx, cy, color);
            drawCatSymParts(-1, r, cx, cy, color);

            // Nose
            tft->drawTriangle(cx-0.07*r, cy+0.1, cx+0.07*r, cy+0.1, cx, cy+0.2*r, color);

            // Mouth
            tft->startWrite();
            tft->drawCircleHelper(cx-0.2*r, cy+0.2*r, 0.2*r, 4, color);
            tft->drawCircleHelper(cx+0.2*r, cy+0.2*r, 0.2*r, 8, color);
            tft->endWrite();
        }

        void drawCatSymParts(const int sign, const byte r, const byte cx, const byte cy, const uint16_t color) {
            // Ear
            tft->drawLine(cx-0.866*r*sign, cy-0.5*r, cx-0.9*r*sign, cy-1.5*r, color);
            tft->drawLine(cx-0.5*r*sign, cy-0.866*r, cx-0.9*r*sign, cy-1.5*r, color);

            // Whiskers
            tft->drawLine(cx-0.3*r*sign, cy-0.10*r+0.2*r, cx-1.3*r*sign, cy-0.2*r, color);
            tft->drawLine(cx-0.3*r*sign, cy-0.0*r+0.2*r, cx-1.3*r*sign, cy+0.1*r, color);
            tft->drawLine(cx-0.3*r*sign, cy+0.10*r+0.2*r, cx-1.3*r*sign, cy+0.4*r, color);

            // Eyes
            tft->drawRoundRect(cx-0.4*r*sign-0.15*r, cy-0.3*r-0.1*r, 0.3*r, 0.2*r, 0.1*r, color);
        }


    public:
        ScreenOut(Adafruit_ST7735 *tft) : tft(tft) {
        }


        void begin() {
            tft->initR(INITR_144GREENTAB);
            tft->setRotation(3);
            tft->fillScreen(ST77XX_BLACK);
            drawCat();
        }

        void writeNormalValues(const char* mainText, const int switchPos, const int switchOhms, const int motorPos, const float motorVolts) {
            char buffer[maxChars];

            // Fill normal layout with values
            if (currentLayout != 1) {
                initNormalLayout();
                resetStored();
            }

            if (switchPos != currentSwitchPos) {
                DEBUG_PRINT(F("ScreenOut>WriteNormalValues: switchPos = ")); DEBUG_PRINT(switchPos); DEBUG_PRINTLN("");
                posToStr(buffer, switchPos);
                writeBlock(buffer, 4, 25, 2, SCREEN_WIDTH/2-8, 1);
                currentSwitchPos = switchPos;
            }

            if (motorPos != currentMotorPos) {
                posToStr(buffer, motorPos);
                writeBlock(buffer, SCREEN_WIDTH/2+4, 25, 2, SCREEN_WIDTH/2-8, 1);
                currentMotorPos = motorPos;
            }

            if (abs(switchOhms - currentSwitchOhms) > 10) {
                sprintf(buffer, "%d \351", switchOhms);
                writeBlock(buffer, 4, 50, 1, SCREEN_WIDTH/2-8, 1);
                currentSwitchOhms = switchOhms;
            }

            if (abs(motorVolts - currentMotorVolts) > 0.02) {
                char temp[6];
                dtostrf(motorVolts, 4, 3, temp);
                sprintf(buffer, "%s V", temp);
                writeBlock(buffer, SCREEN_WIDTH/2+4, 50, 1, SCREEN_WIDTH/2-8, 1);
                currentMotorVolts = motorVolts;
            }

            if (strcmp(mainText, currentMainText) != 0) {
                writeBlock(mainText, 4, 75, 1, SCREEN_WIDTH-8, 4);
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
        float motorVolts = -1;
        int displayMode = 0;  // So screen can display different information based on selected mode
        // char motorMessage[33]; // Message from Motor
        ScreenOut screenOut;

        void writeDisplay() {
            // screenOut.writeScreen(mainMessage, switchPos, motorPos);
            screenOut.writeNormalValues(mainMessage, switchPos, switchResistance, motorPos, motorVolts);
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
            copystr(mainMessage, message, maxChars*4);
            writeOutputs();
        }

        void setMainMessage (const __FlashStringHelper *message) {
            // const char *buffer = (const char PROGMEM *)message;
            copystr(mainMessage, message, maxChars*4);
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

        // void setMotorMessage(const char *message) {
        //     copystr(motorMessage, message, 16);
        //     writeOutputs();
        // }

        // void setMotorMessage(const __FlashStringHelper *message) {
        //     copystr(mainMessage, message, 16);
        //     writeOutputs();
        // }

        void showCat() {
            screenOut.showCat();
        }
};