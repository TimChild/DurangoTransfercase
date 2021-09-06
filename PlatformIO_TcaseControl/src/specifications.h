#pragma once

// Switch resistance specs
int SW_SHORTED_HIGH = 150;
int SW_N_AWD_LOW = 176;
int SW_N_AWD_HIGH = 200;
int SW_N_LOCK_LOW = 190;
int SW_N_LOCK_HIGH = 216;
int SW_N_LO_LOW = 199;
int SW_N_LO_HIGH = 226;
int SW_AWD_LOW = 1159;
int SW_AWD_HIGH = 1287;
int SW_LOCK_LOW = 2259;
int SW_LOCK_HIGH = 2503;
int SW_LO_LOW = 4820;
int SW_LO_HIGH = 5334;
int SW_OPEN_LOW = 19000;

// Switch debounce time (s)
float SW_DEBOUNCE_S = 0.25;


// Mode sensor voltages
// Positions {0, 1, 2, 3} == 4HIGH, AWD, Neutral, 4LO
float LOCK_LOW = 4.26;
float LOCK_HIGH = 4.36;
float AWD_LOW = 3.36;
float AWD_HIGH = 3.44;
float N_LOW = 2.45;
float N_HIGH = 2.54;
float LO_LOW = 1.48;
float LO_HIGH = 1.57;
float LOW_LIMIT = 0.50;
float HIGH_LIMIT = 4.51;
// TODO: Note to self, shifting from 4LO to AWD first shifts from 4LO to 4HIGH then back to AWD

// Shift parameters
float MAX_SHIFT_TIME_S = 1.0;  // Max time to try shifting per position change
int MAX_SINGLE_SHIFT_ATTEMPTS = 3;  // Max times to try shifting before shifting back
float INTERRUPT_TIME_S = 0.2;  // Time to wait before reversing direction of failed shift
float RETRY_TIME_S = 2.0;  // Time to wait before retrying a shift
float POSITION_TOLERANCE = 0.05;  // Stop shifting once within this distance of target voltage

// From Service Manual:
// "Current attempt limit values are 25 transitions in 30 seconds and default mode
// values are 3 transitions every 15 seconds for 5 minutes."
// Note: Goes into "default mode" if 25 transitions in 30s (i.e. default mode is to cool down)

int CUMULATIVE_SHIFT_TIME_S = 30; // Count up attempted shifts in last X seconds
int MAX_SHIFT_ATTEMPTS_BEFORE_OVERHEAT = 15; // (30 in SM, but counts blocks as 2) Number of shift attempts allowed within CUMULATIVE_SHIFT_TIME
int MOTOR_COOLDOWN_TIME_S = 300; // During this time, shifts should delay should be increased to 5s
float MOTOR_COOLDOWN_ADDITIONAL_DELAY_S = 5.0; // delay shifts by additional 5s to allow motor to cool down
// Note: Service Manual definces some very complicated procedures for shifting in and out of Neutral if shifts are blocked.
// I think we will stick to some more simple procedures


// Drift correction parameters
float CHECK_INTERVAL_S = 2.0; // Time between checks of Drift


// PWM parameters
int PWM_FREQUENCY = 100; // 100Hz PWM Frequency
float PWM_ACCELERATION = 0.05; // Not specified in manual (only says "specified rate"):
                              // % increase of duty per cycle

// Not specified in manual (says "specified rate based upon difference between desired position and current position")
// TODO: This values need to be figured out!
float PWM_DECELERATION_RELATION = 0.50; // % decrease of duty per cycle (to be multipled by an inverse distance)
float PWM_DECELERATION_DISTANCE = 0.2;  // Voltage distance at which to start decelerating
int PWM_MAX_POWER = 100; // TESTING ONLY: Mostly for testing so that the max power can be reduced (out of 255)
int PWM_MIN_POWER = 10; // Not specified in manual - probably need some minimum power to actually make motor move

// Shift motor braking parameters (apply 12V to both shift motor wires)
int STATIC_BRAKE_POSITIONS[] = {1, 3};  // Static brake in AWD and 4LO

// Shift Brake Release time
int BRAKE_RELEASE_TIME_S = 3;  // should be between 2 - 5 seconds before and after


// Other non-service manual values
int EEPROM_POSITION_ADDRESS = 0;