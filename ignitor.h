#ifndef IGNITOR
#define IGNITOR

#include "common.h"

enum eIgnitorState
{
  eIgnitorIdle,
  eIgnitorStart,
  eIgnitorWait,
  eIgnitorEnd
};

class Ignitor
{
public:  
  eIgnitorState m_state;
  unsigned long IgnitionTime; // time the ignitor was started
  
  void Init();
  void Start();
  void End();
  void Update();
  
};

extern Ignitor m_ignitor; // ignition controller

#endif

