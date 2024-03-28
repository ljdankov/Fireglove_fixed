#ifndef COMMON_VARS
#define COMMON_VARS
#include "settings.h"

extern int SolenoidPWMValue ;// a 0 - 255 value reading of the pot 

#define EEPROM_ADDRESS 0 // the address where we're storing and reading values

#define Solenoid_Input_Pin  A6   // (A6) (INPUT)Analog input pin to read 10K pot value. This is scaled to output a PWM value on pin D9
#define TriggerButtonPin  2    // (D2) (INPUT)the pin number of the trigger button
#define SelectButtonPin  4    // (D4) (INPUT)the pin number of the select button
#define SolenoidPin  5    // (D5)/PWM (OUTPUT)the pin number of the gas solenoid valve driver
#define IgnitionPin  6    // (D6) (OUTPUT)the pin number of the ignition coil driver - maybe PWM?
#define ModeLEDPinR  9      // (D9) (OUTPUT) RED - Single Burst
#define ModeLEDPinG  10      // (D10) (OUTPUT) GREEN - Continous Burst
#define ModeLEDPinB  11      // (D11) (OUTPUT) BLUE - Continous Flame
#define FSR_PIN A5 // (A5) (INPUT)Analog input pin to read force sensitive resistor as a switch
//#define ModeLEDPinR  17      // (D9) (OUTPUT) RED - Single Burst
//#define ModeLEDPinG  18      // (D10) (OUTPUT) GREEN - Continous Burst
//#define ModeLEDPinB  19      // (D11) (OUTPUT) BLUE - Continous Flame
#endif
