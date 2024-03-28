//#include <Arduino.h>
#include "common.h"
#include "burst.h"
#include "ignitor.h"

/*
Shape of solenoid burst:
So, this funny graph deserves some explanation:
The solenoid *kinda can be PWM controlled with values from 0->255 from the Arduino 8-bit DAC
It requires an initial high PWM value (near full val of 220 - 255) to initially open the valve.
After the valve is opened (B), the PWM can be scaled back to a lower value (C) and still be held partially open 
It can then be ramped up to the desired value from ( C->D ) which is the Max value used for this burst.
The time period from C->D is based on the acceleration of the gas to allow for ignition.
The time period from A->C (eInitialOpen) can probably be fairly short (20mS - 25mS according to specs of the solenoid response time)
The solenoid PWM value is then held at level D/E until the burst period is over and then dropped down to 0 (F) to turn off again

PWM 
255
       B_ 
       | |
       | |   D________E
       | |  /         |
       | | /          |
       | |/           |
       | C            |
       |              |
  -----A              F---------
0
TIME ---------->
*/
int valtoggle = LOW;
unsigned long togltime = 0;
/*
This function is called at application start-up
*/
void Burst::Init()
{
  m_state = eIdle;  
  //BurstTime = 0;  
  pinMode(SolenoidPin, OUTPUT);  // set up the pin for the gas solenoid valve
  digitalWrite(SolenoidPin, LOW); // set it to off
  pinMode(ModeLEDPinR, OUTPUT);  // set up the RED
  analogWrite(ModeLEDPinR, LOW); // set it to off
  pinMode(ModeLEDPinG, OUTPUT);  // set up the Green
  analogWrite(ModeLEDPinG, LOW); // set it to off  
  pinMode(ModeLEDPinB, OUTPUT);  // set up the Blue
  analogWrite(ModeLEDPinB, LOW); // set it to off
  Serial.println("Burst Setup complete");
  SetMode(eSingleBurst);
}

// This is the function used to start the burst sequence
void Burst::Start()
{
  m_state = eWaitForDone; //set the state machine
  BurstTime = millis(); // mark when we started
  togltime = millis();
}

/*
This is called to end a burst 
*/
void Burst::End() 
{
  analogWrite(SolenoidPin,0); // turn off the solenoid valve
  m_state = eIdle; // move the state machine back to idle
}
/*
This is the main state machine that uses time to go to the next state
We initially spike the valve at full value to make sure it's open, then reduce it to the starting value quickly
We then use an acceleration value to the solenoid PWM,
What this means is that we'll start to emit gas slow, and accelerate the output rate up to the max
value over a short period of time
this will allow the gas to ignite prior to achieving maximum flow rate
The SolenoidPWMValue controlled by the potentiometer is the Max flow rate of the gas through the solenoid
*/
void Burst::Update()
{
  unsigned long timenow = millis(); // get the time now
  UpdateModeLED(); // update the pulsing led  
 
  switch(m_state)
  {
    case eIdle: // do nothing      
    break; 
    case eWaitForDone:
      
        if(timenow > togltime)
        {
          // next mod is to add some basic PWM in here to control the on/off ratio instead of the speed
          if(valtoggle == LOW)
          {
            digitalWrite(SolenoidPin,valtoggle); // Set the solenoid PWM valve                                
            valtoggle = HIGH;
            togltime = timenow + 11 + SolenoidPWMValue;
          }
          else // high
          {
            digitalWrite(SolenoidPin,valtoggle); // Set the solenoid PWM valve                                
            valtoggle = LOW;
            togltime = timenow + 11 + (-1 * SolenoidPWMValue);
          }
          
        }
        // check to see if we're past the time
        if(timenow > (BurstTime + pVars.burst_duration))
        {
          switch(Mode())
          {
            case eContinousFlame: // do nothing, user must end manually by letting go of the button
            break;
            case eContinousBurst:
                //go to the rest state between bursts  
                analogWrite(SolenoidPin,0); // Set the solenoid PWM valve                       
                m_state = eRest; // move the the next state
                //restart the timer for the rest period
                BurstTime = millis();               
            break;
            case eSingleBurst:
                analogWrite(SolenoidPin,0); // Set the solenoid PWM valve        
                m_state = eIdle;//we're done here, go back to idle  
            break;
          }
        }
    break;
    case eRest:
        if(timenow > (BurstTime + pVars.burst_delay))
        {
          Start(); // start another burst
          //we need to start the ignitor too   
          m_ignitor.Start();            
        }      
    break;
    }  
}

/*
This function will cycle through to the next mode
*/
void Burst::NextMode()
{
  switch (m_mode)
    {
      case eSingleBurst: 
        SetMode(eContinousBurst);
        break;
      case eContinousBurst: 
        SetMode(eContinousFlame);
        break;
      case eContinousFlame: 
        SetMode(eSingleBurst);
        break;
    }      
}

void Burst::SetMode(eFlameMode mode)
{
  m_mode = mode;
  //set all the leds low - the UpdateModeLED will take care of colors
  analogWrite(ModeLEDPinR,0);
  analogWrite(ModeLEDPinG,0);
  analogWrite(ModeLEDPinB,0);  
}

void Burst::PrintMode()
{
  Serial.print("Current Burst Mode : ");  
  switch (m_mode)
  {
    case eSingleBurst: // a single fireball burst of duration X will occur
      Serial.println("Single Burst");
      break;
    case eContinousBurst: // a fireball burst of duration X will occur with a delay of Y between bursts
      Serial.println("Continous Burst");
      break;
    case eContinousFlame: // the flame will be continous
      Serial.println("Continous Flame");
      break;
  }  
}

// make the LED pulse the correct color based on the time 
// this will pulse the led once per sec
// using the min / max values for each led stored in the eeprom

void Burst::UpdateModeLED() 
{
  unsigned long timenow = millis(); // get the current time in mS    
  int ms = timenow % 1000; // take the 0 -> 999 modulus remainder
  int odd = 0; // assume we're on an even second 
  unsigned long timesec = timenow / 1000; // integer division to get the seconds 
  
  if(timesec & 1) // if the LSB is 1, then this is an odd number
    odd = 1; 
    
  int pin = ModeLEDPinR; // give it a default
  int valmax=MAX_LED; //default
  //figure out which pin and max we're working with
  
  switch (m_mode)
  {
    case eSingleBurst: 
      pin = ModeLEDPinB;
      break;
    case eContinousBurst:
      pin = ModeLEDPinR;
      break;
    case eContinousFlame: 
      pin = ModeLEDPinG;
      break;
  }     

  // ok, now we have everything we need
  int ledval = 0;
  if(odd)
  {
      ledval = map(ms,0,1000,valmax,0); // go down in brightness
  }else
  {
      ledval = map(ms,0,1000,0,valmax); // increase in brightness
  }
  //write the value to the LED
  analogWrite(pin,ledval); 
}
