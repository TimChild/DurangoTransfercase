#pragma once
#include <Arduino.h>
#include <Wire.h>  // For LiquidCrystal
#include <LiquidCrystal.h>


String makeStringFixedLen(String str, int len) {
    // Returns a string with a spaces added to reach len
        str = str.substring(0, len); // Make sure only max len chars
        for(int i = int(str.length()); i < len; i++) {
            str += ' ';  
        }
    return str;
}

class OtherOutputs {
    private: 
        String mainMessage = "";  // Main message text
        int switchPos = -1; 
        int motorPos = -1;
        float motorVolts = -1;
        String motorMessage = ""; // Message from Motor
        LiquidCrystal screen;

        void writeDisplay() {
            // Combine the different info into a message to display on screen
            String displayText;
            // screen.clear();
            // screen.autoscroll();  // Something like this might be useful
            // screen.noAutoscroll(); // And this... might need some delays while writing etc
            // screen.clear();
            displayText = makeStringFixedLen(mainMessage, 16);
            screen.setCursor(0,0);
            screen.print(displayText);
            
            displayText = makeStringFixedLen(String(switchPos), 4);
            displayText += makeStringFixedLen(String(motorPos), 12);
            screen.setCursor(0,1);
            screen.print(displayText);
        }        

        void writeFakePinOuts() {
            // Set pin outs to trick the Car into thinking it's in a certain state
            // TODO: Set pin outs (probably using pwm analog out with a lowpass filter?)
        }

    public:
        OtherOutputs() : screen(0,0,0,0,0,0) {
        } // TODO: Almost definitely wrong and needs fixing

        // OtherOutputs(LiquidCrystal lcd) : screen(lcd) {
        //     screen.begin(16, 2);
        // }

        void setLcd(LiquidCrystal lcd) {
            screen = lcd;
            screen.begin(16,2);
            screen.clear();
        }

        void writeOutputs() {
            // Output signals to trick car into thinking it's in correct state
            // And display screen
            writeFakePinOuts();
            writeDisplay();
        }

        void setBrightness () {
            // Set brightness of screen (might not be supported)
            // TODO: Set brightness if possible
        }

        void setMainMessage (String message) {
            mainMessage = message;
            writeOutputs();
        }

        void setSwitchPos(int pos) {
            if (0 <= pos && pos <= 3) {
                switchPos = pos;
            }
            writeOutputs();
        }

        void setMotorPos(int pos) {
            if (0 <= pos && pos <= 3) {
                motorPos = pos;
            }
            writeOutputs();
        }

        void setMotorMessage(String message) {
            motorMessage = message;
            writeOutputs();
        }

        void setMotorVolts(float volts) {
            motorVolts = volts;
            writeOutputs();
        }
        
};