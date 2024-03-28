#include <Arduino.h>
#include "ignitor.h"

void Ignitor::Init()
{
  pinMode(IgnitionPin, OUTPUT);  // set up the pin for the ignition coil
  digitalWrite(IgnitionPin, LOW); // set it to off
  m_state = eIgnitorIdle;
  IgnitionTime = 0;
  Serial.println("Iggnition Intitialized1");
}
void Ignitor::Start()
{
  m_state = eIgnitorStart; // go to start state
  IgnitionTime = millis();
}
void Ignitor::End()
{
  //Serial.println("ignition end");
  digitalWrite(IgnitionPin,LOW); // turn off the ignition if on  
  m_state = eIgnitorIdle;
}
void Ignitor::Update()
{
  unsigned long timenow = millis();
  switch(m_state)
  {
    case eIgnitorIdle:
      //do nothing
    break;
    case eIgnitorStart:
    Serial.println("ignition start");
      digitalWrite(IgnitionPin, HIGH); // set it to high    
      m_state = eIgnitorWait;  // go to the wait state
    break;
    case eIgnitorWait:
      if(timenow > (IgnitionTime + pVars.ignition_duration)) // check to see if the ignition duration is ended
      {
        m_state = eIgnitorEnd; // move the end state to stop the ignition
      }    
    break;
    case eIgnitorEnd:
      End(); // turn off ignition
    break;
  }
}
