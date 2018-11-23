extern void ToggleOnOff();
extern void requestOn();
extern void requestOff();
extern int  getRunState();
extern void reqTempChange(int val);
extern void tempDec();
extern int  getSetTemp();
extern void reqDisplayUpdate();
extern void reqThermoToggle();
extern void setThermostatMode(unsigned char);
extern bool getThermostatMode();
extern float getFixedHz();

extern float fFilteredTemperature;
