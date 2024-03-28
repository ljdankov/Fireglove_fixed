#include <Arduino.h>
#include "settings.h"
#include <EEPROM.h>

#define EEPROM_KEY_VALUE 345 // change this when adding to the AppVars struct

//set default values
void DefVals()
{
  pVars.burst_duration = 350;
  pVars.ignition_duration = 150;
  pVars.burst_delay = 100; // miilisecond between bursts
  pVars.solenoid_pot_min = 0; // this min value of the pot (0-1023)
  pVars.solenoid_pot_max = 1023; // this max value of the pot (0-1023)
  //24 with an accelaration of 768 seems like it gives 2 bursts. I think we need to increase the pwm min
  pVars.validkey = 0;
}

void SaveVars()
{
  int eeAddress = 0;
  pVars.validkey = EEPROM_KEY_VALUE;
  EEPROM.put(eeAddress, pVars);
}

void LoadVars()
{
//  int eeAddress = 0;
//  EEPROM.get(eeAddress, pVars);
//  if (pVars.validkey != EEPROM_KEY_VALUE)
//  {
    DefVals();//no data, set default vals
//  }
}

