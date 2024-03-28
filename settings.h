#ifndef SETTINGS
#define SETTINGS
/*
  A structure to hold application variables, and to make it easier to save and load them from EEPROM
*/

#define SOLENOID_INITIAL_DURATION 35
//#define SOLENOID_MIN_DURATION 55
#define SOLENOID_MIN_DURATION 11

#define MAX_LED 64
#define SOLENOID_ACCEL 768 // 1024
#define SHORT_BURST 200
#define LONG_BURST 750

struct AppVars
{
  int burst_duration; // how long to make a fireball burst last (milliseconds)
  int ignition_duration; // how long should the ignition happen for
  int burst_delay; // how long to wait between bursts
  int solenoid_pot_min; // this min value of the pot (0-1023)
  int solenoid_pot_max; // this max value of the pot (0-1023)
  int validkey; // is this valid data
};

extern AppVars pVars; // global application variables

void DefVals();
void SaveVars();
void LoadVars();
#endif
