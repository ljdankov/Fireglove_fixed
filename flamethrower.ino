/*
  This is the firmware for my wrist-mounted flamethrower
  Hopefully, this isn't all overkill
*/
#include <EEPROM.h>
#include "common.h"
#include "ignitor.h"
#include "burst.h"
#include "settings.h"

Burst m_burst; // burst controller
Ignitor m_ignitor; // ignition controller

// some variables for receiveing and parsing from the user:
char message[50];
int mess_pos = 0;
char incomingByte = 0;

int TriggerButtonState = LOW;       // the current reading from the input pin
int lastTriggerButtonState = LOW;   // the previous reading from the input pin

int SelectButtonState = LOW;       // the current reading from the input pin
int lastSelectButtonState = LOW;   // the previous reading from the input pin

unsigned long lastTriggerDebounceTime = 0;  // the last time the trigger pin was toggled
unsigned long lastSelectDebounceTime = 0;  // the last time the trigger pin was toggled
#define  debounceDelay 50                 // the debounce time in milliseconds; increase if the output flickers


void setup()
{
  Serial.begin(115200); // set up the serial port
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  WelcomeMessage();
  PrintArgs(); // print the arguments
  LoadVars(); // load vars from eeprom
  SetupIO();
  m_burst.Init();
  m_ignitor.Init();
}

void SetupIO()
{
  pinMode(TriggerButtonPin, INPUT_PULLUP);  // inverted - button pressed = low, not pressed = high
  pinMode(SelectButtonPin, INPUT_PULLUP);  // inverted - button pressed = low, not pressed = high
  pinMode(Solenoid_Input_Pin, INPUT);  // solenoid input pin -  necessary?
  pinMode(FSR_PIN,INPUT);
}

void DoTriggerInput()
{
  //int reading = !digitalRead(TriggerButtonPin); //read and invert the trigger button pin, if pressed will be high, if not pressed will be low
  int reading=0;
  int freading = analogRead(FSR_PIN);
  if(freading>10){Serial.println("Raw Val : " + String(freading));}
  if (freading >800)
    {reading=HIGH;
    lastTriggerDebounceTime=millis();}
  else 
  {reading=LOW;}
  if (reading != TriggerButtonState)  // check for change in trigger button state, either on->off or off->on
    {TriggerButtonState = reading;
      if (TriggerButtonState == HIGH) // the button went from off to on for the first time
      {// just transitioned high
        Serial.println("Triggered");
        m_burst.Start(); // start the burst of gas
        m_ignitor.Start(); // start the ignitor
      }
      else if (TriggerButtonState == LOW) // the button just went from on to off for the first time
      {if(m_burst.Mode() == eContinousFlame || m_burst.Mode() == eContinousBurst)// if we're in continous flame mode or continous burst mode, then we stop when the user lets go of the button 
        {m_burst.End(); // manual end of flame / solenoid
          m_ignitor.End(); //manual end of ignitor, could have already ended at this point due to timeout    
        }
      }
    }
  //save the reading. next time through the loop the state of the reading will be known as the lastTriggerButtonState
  lastTriggerButtonState = reading;
}
void DoModeInput(){
  int reading = !digitalRead(SelectButtonPin); //read and invert the select button pin, if pressed will be high, if not pressed will be low
  if (reading != lastSelectButtonState) {lastSelectDebounceTime = millis();} //if the reading is not equal to the last state set the last debounce time to current millis time
  //check the difference between current time and last registered button press time, if it's greater than user defined delay then change the state as it's not a bounce
  if ((millis() - lastSelectDebounceTime) > debounceDelay)
  {if (reading != SelectButtonState)  // check for change
    {SelectButtonState = reading;
      if (SelectButtonState == HIGH){Serial.println("ModeSwitch"); m_burst.NextMode();}// just transitioned high
    }
  }
  //save the reading. next time through the loop the state of the reading will be known as the lastTriggerButtonState
  lastSelectButtonState = reading;
}

void WelcomeMessage()
{ 
  Serial.println("************************************");
  Serial.println("Wrist-Mounted Flame-Thrower Firmware");
  Serial.println("Version 1.0.0.5 : 7.20.2018");
  Serial.println("PacManFan");
  Serial.println("pacmanfan321@gmail.com");
  Serial.println("Warrenty : Use at your own damn risk!"); 
}

void PrintArgs()
{
  Serial.println("***************************************");  
  Serial.println("Commands: ");
  Serial.println("B <val> -Burst Duration (mS)");
  Serial.println("I <val> -Ignitor Duration (mS)");
  Serial.println("D <val> -Delay between bursts"); 
  Serial.println("T <val> -Solenoid Minimum On Value (0-255)"); 
  Serial.println("C - Do calibration for solenoid control");
  Serial.println("N - Next Mode");
  Serial.println("P - Print Live Status");
  Serial.println("E - Print EEPROM Values");
  Serial.println("S - Save Current values");  
  Serial.println("Z - Trigger ignition sequence");  
  Serial.println("X - Trigger burst sequence");  
//  Serial.println("A - Stop All");    
}

void Print_EEPROM_Vals()
{
  Serial.println("*****EEPROM Values:");   
  Serial.print("Burst Duration (mS) : ");
  Serial.println(pVars.burst_duration);
  Serial.print("Ignition Duration (mS) : ");
  Serial.println(pVars.ignition_duration);
  Serial.print("Delay between Bursts (mS): ");
  Serial.println(pVars.burst_delay);
  Serial.print("Minimum POT Solenoid value: ");
  Serial.println(pVars.solenoid_pot_min);
  Serial.print("Maximum POT Solenoid value: ");
  Serial.println(pVars.solenoid_pot_max);
  Serial.print("Solenoid Initial Duration: ");
  Serial.println(SOLENOID_INITIAL_DURATION);
  Serial.print("Solenoid Gas acceleration: ");
  Serial.println(SOLENOID_ACCEL);
  Serial.print("Max LED brightness: ");
  Serial.println(MAX_LED);  
}

void PrintVals()
{
  //now take a reading from the solenoid analog input, and show it
  Serial.print("Raw POT analog value (0 - 1023): ");
  Serial.println(analogRead(Solenoid_Input_Pin));
  Serial.print("scaled PWM analog value: ");
  Serial.println(SolenoidPWMValue);    
  m_burst.PrintMode();
  //print button states
  Serial.print("Trigger Button State: ");
  Serial.println(TriggerButtonState);    
  Serial.print("Mode Select Button State: ");
  Serial.println(SelectButtonState);    
  
}

// parse the complete recieved message
void ParseMessage()
{
  Serial.println(message);
  char func = message[0];
  func = toupper(func); // convert to upper case
  switch (func)
  {
    case 'B': // duration of burst
      sscanf((const char *)&message[2], "%d", &pVars.burst_duration);
      Print_EEPROM_Vals();
      break;
    case 'I': // duration of the ignition
      sscanf((const char *)&message[2], "%d", &pVars.ignition_duration);
      Print_EEPROM_Vals();
      break;
    case 'D':
      sscanf((const char *)&message[2], "%d", &pVars.burst_delay);
      Print_EEPROM_Vals();
      break;
//    case 'Y':
//      sscanf((const char *)&message[2], "%d", &pVars.solenoid_Initial_Duration);
//      Print_EEPROM_Vals();
//      break;
//    case 'U':
//      sscanf((const char *)&message[2], "%d", &pVars.solenoid_acceleration);
//      Print_EEPROM_Vals();
//      break;
//    case 'O':
//      sscanf((const char *)&message[2], "%d", &pVars.maxLED);
//      Print_EEPROM_Vals();
//      break;
    case 'C':
      CalibratePotInput(); // do calibration
      PrintVals();
      break;
    case 'N': // Next Mode
      m_burst.NextMode();
      break;
    case 'S': // save data
      SaveVars();
      break;
    case 'P':
      PrintVals();
      break;
    case 'E':
      Print_EEPROM_Vals();
      break;
    case 'Z':
      m_ignitor.Start(); // trigger the ignition sequence
      break;
    case 'X':
      m_burst.Start(); // trigger the burst sequence
      break;
    case 'A':
      m_burst.End(); // End the burst if active
      m_ignitor.End(); // end the ingition if active
      break;
    default:
      PrintArgs();
      break;
  }
}
/*
  This function reads from the serial if available and runs the command
*/
void DoSerial()
{
  if (Serial.available() > 0)
  {
    incomingByte = Serial.read();
    if (incomingByte == '\r')
    {
      //end of message
      Serial.println("Recieved");
      message[mess_pos++] = 0; // terminate the string
      mess_pos = 0; //reset back to first position
      ParseMessage();
    } 
    else
    {
      message[mess_pos++] = incomingByte; // store next incoming byte
    }
  }
}
// small calibration routine
void CalibratePotInput()
{
  //eat any input
  while (Serial.available() > 0)Serial.read();
  Serial.println("******** Analog solenoid potentiometer input calibration routine: ");
  Serial.println("Move the sensor to the MINIMUM position and press any key");
  while (Serial.available() > 0){} // do nothing
  pVars.solenoid_pot_min = analogRead(Solenoid_Input_Pin);
  Serial.println("Move the sensor to the MAXIMUM position and press any key");
  while (Serial.available() > 0){} // do nothing
  pVars.solenoid_pot_max = analogRead(Solenoid_Input_Pin);
  //we really need to determine the minimum PWM 'trip' value - this is the value that is the minimum for 
  //gas to start flowing
  //solenoid_PWM_min; // the minimum value at which the PWM will trip the solenoid (0-255)
  Serial.print("Min: ");
  Serial.println(pVars.solenoid_pot_min);
  Serial.print("Max: ");
  Serial.println(pVars.solenoid_pot_max);
  Serial.println("Remember to save values to EEPROM with the 'S' command.");   
}

void DoPotInput()
{
  //read the 10K potentiometer   
  // translate and scale the value
  //set the PWM value for the solenoid
  //SolenoidPWMValue = map(analogRead(Solenoid_Input_Pin),0,1023,pVars.solenoid_PWM_min,255);
  SolenoidPWMValue = map(analogRead(Solenoid_Input_Pin),0,1023,-SOLENOID_MIN_DURATION,SOLENOID_MIN_DURATION);
}

void loop()
{
  DoSerial(); // receive and process commands from serial
  DoTriggerInput(); // handle the trigger button
  DoModeInput(); // handle the select button
  DoPotInput(); // read the analog potentiometer value and set the SolenoidPWMValue variable 
  m_burst.Update(); // update the state machine for the burst controller
  m_ignitor.Update(); // update the state machine for the ignition controller
}
