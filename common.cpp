
// constants won't change. They're used here to set pin numbers:

int SolenoidPWMValue = 255 ;// full on for now
struct AppVars
{
  int burst_duration; // how long to make a fireball burst last (milliseconds)
  int ignition_duration; // how long should the ignition happen for
  int burst_delay; // how long to wait between bursts
  int validkey; // is this valid data
};

AppVars pVars; // global application variables

