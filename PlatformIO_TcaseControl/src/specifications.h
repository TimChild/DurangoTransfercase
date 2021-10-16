#pragma once

#define FOURHI 0
#define AWD 1
#define NEUTRAL 2
#define FOURLO 3

// All for NV244 transfercase

// Switch resistance specs
const int SW_SHORTED_HIGH = 150;
const int SW_N_AWD_LOW = 176;
const int SW_N_AWD_HIGH = 200;
const int SW_N_LOCK_LOW = 190;
const int SW_N_LOCK_HIGH = 216;
const int SW_N_LO_LOW = 199;
const int SW_N_LO_HIGH = 226;
const int SW_AWD_LOW = 1159;
const int SW_AWD_HIGH = 1287;
const int SW_LOCK_LOW = 2259;
const int SW_LOCK_HIGH = 2503;
const int SW_LO_LOW = 4820;
const int SW_LO_HIGH = 5334;
const int SW_OPEN_LOW = 19000;
// NV144: Same as NV244 but only 4HI and AWD

// Switch debounce time (s)
const float SW_DEBOUNCE_S = 0.25;
const float SW_N_PRESS_TIME_S = 3.0;


// Mode sensor voltages
// Positions {0, 1, 2, 3} == 4HIGH, AWD, Neutral, 4LO
const float LOCK_LOW = 4.26;
const float LOCK_HIGH = 4.36;
const float AWD_LOW = 3.36;
const float AWD_HIGH = 3.44;
const float N_LOW = 2.45;
const float N_HIGH = 2.54;
const float LO_LOW = 1.48;
const float LO_HIGH = 1.57;
const float LOW_LIMIT = 0.50;
const float HIGH_LIMIT = 4.51;
// TODO: Note to self, shifting from 4LO to AWD first shifts from 4LO to 4HIGH then back to AWD
// NV144: 
// 4HI = 4.19 -> 4.35
// AWD = 0.45 -> 0.55

// Shift parameters
// float MAX_SHIFT_TIME_S = 1.0;  // Max time to try shifting per position change
const float MAX_SHIFT_TIME_S = 3.0;  // Max time to try shifting per position change
const byte MAX_SINGLE_SHIFT_ATTEMPTS = 3;  // Max times to try shifting before shifting back
// const float INTERRUPT_TIME_S = 0.2;  // Time to wait before reversing direction of failed shift
// const float RETRY_TIME_S = 2.0;  // Time to wait before retrying a shift
const float POSITION_TOLERANCE = 0.01;  // Stop shifting once within this distance of target voltage

// From Service Manual:
// "Current attempt limit values are 25 transitions in 30 seconds and default mode
// values are 3 transitions every 15 seconds for 5 minutes."
// Note: Goes into "default mode" if 25 transitions in 30s (i.e. default mode is to cool down)

// const int CUMULATIVE_SHIFT_TIME_S = 30; // Count up attempted shifts in last X seconds
// const int MAX_SHIFT_ATTEMPTS_BEFORE_OVERHEAT = 15; // (30 in SM, but counts blocks as 2) Number of shift attempts allowed within CUMULATIVE_SHIFT_TIME
// const int MOTOR_COOLDOWN_TIME_S = 300; // During this time, shifts should delay should be increased to 5s
// const float MOTOR_COOLDOWN_ADDITIONAL_DELAY_S = 5.0; // delay shifts by additional 5s to allow motor to cool down
// Note: Service Manual definces some very complicated procedures for shifting in and out of Neutral if shifts are blocked.
// I think we will stick to some more simple procedures


// Drift correction parameters
// const float CHECK_INTERVAL_S = 2.0; // Time between checks of Drift


// PWM parameters
const int PWM_FREQUENCY = 490; // FCM uses 100Hz PWM Frequency but Arduino uses 490Hz by default (not worth changing)
const float PWM_ACCELERATION = 2.0; // Not specified in manual (only says "specified rate"):
                           // increase of duty cycle per second (i.e. duty == 1.0 is MAX so 2.0 means 0 -> MAX in 0.5s)

// Not specified in manual (says "specified rate based upon difference between desired position and current position")
// float PWM_DECELERATION_RELATION = 0.50; // % decrease of duty per cycle (to be multipled by an inverse distance)
// const float PWM_DECELERATION_DISTANCE = 0.2;  // Voltage distance at which to start decelerating
const byte PWM_MAX_POWER = 200; // TESTING ONLY: Mostly for testing so that the max power can be reduced (out of 255)
const byte PWM_MIN_POWER = 15; // Not specified in manual - probably need some minimum power to actually make motor move

// Shift motor braking parameters (apply 12V to both shift motor wires)
// const byte STATIC_BRAKE_POSITIONS[] = {1, 3};  // Static brake in AWD and 4LO

// Shift Brake Release time
const byte BRAKE_RELEASE_TIME_S = 3;  // should be between 2 - 5 seconds before and after


// Other non-service manual values
const byte EEPROM_POSITION_ADDRESS = 0;