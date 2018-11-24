extern void ToggleOnOff();
extern void requestOn();
extern void requestOff();
extern int  getRunState();
extern int  getErrState();
extern void reqTempChange(int val);
extern int  getSetTemp();
extern void reqDisplayUpdate();
extern void reqThermoToggle();
extern void setThermostatMode(unsigned char);
extern bool getThermostatMode();
extern float getFixedHz();
extern float getPumpHz();
extern void reqPumpPrime(bool on);

extern float fFilteredTemperature;

#define LOWERLIMIT(A, B) if(A < B) A = B
#define UPPERLIMIT(A, B) if(A > B) A = B