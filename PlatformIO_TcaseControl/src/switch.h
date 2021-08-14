#pragma once
#include <Arduino.h>
#include "output.h"


class SelectorSwitch {
    public:
        int lastValidState;
        int currentState;
        float timeEnteredState;
        OtherOutputs output;

    int getSelection() {
        // Return current selection (last validState after calling check)
        checkState();
        return lastValidState;
    }
    
    void checkState() {
        // Check the switch position. If new update current State and set timeEnteredState
        // If timeEnteredState > X set lastValidState
        output.setSwitchPos(lastValidState);
        output.writeOutputs();
    }
};




/**
 * Read position of selector switch
 */ 
int ReadSwitchPosition() {
  //   return 1;
  return 1;
}



/**
 * Either return new valid position or previous position passed in
 */
int updateValidPosition(int pastValidPosition) {
  int currentPosition;
  currentPosition = ReadSwitchPosition();
  if (0 <= currentPosition && currentPosition <= 3) {
    return currentPosition;
  } else {
    return pastValidPosition;
  }  
}