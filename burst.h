#ifndef BURST
#define BURST

#include "common.h"

/*
 
 This class controls the solenoid to perform bursts or continous flame
 it also controls the RGB LED to indicate the current mode
  
  We're going to cycle through 3 modes,
  1 ) Single Fireball Burst (Red)
  2 ) Continous Burst mode (Green)
  3 ) Continous flame (Blue)

  A single common cathode LED is used to indicate mode (Common ground)
*/

enum eFlameMode
{
  eSingleBurst,  // a single fireball burst of duration X will occur
  eContinousBurst, // a fireball burst of duration X will occur with a delay of Y between bursts
  eContinousFlame // the flame will be continous, ignition is still time-controlled
};

/*A state machine to track the burst state
This makes it easier to do multiple bursts*/
enum eBurstState
{
  eIdle, // no bursting
  eStart, // burst state just started
  eInitialOpen, // The initial period of time to get the solenoid to open
  eWaitForDone, //waiting for the burst to end and ramping up gas via acceleration 
  eRest // rest between multiple bursts
};

class Burst
{
public:
  unsigned long BurstTime;
  eFlameMode m_mode;
  eBurstState m_state;
  void Init();
  void Start();
  void Update();
  void End(); // for manual ending of the burst
  void NextMode();
  void SetMode(eFlameMode mode);
  void PrintMode();
  eFlameMode Mode(){return m_mode;}
private:
  void UpdateModeLED(); // make them pulse based on the time  
};

#endif

