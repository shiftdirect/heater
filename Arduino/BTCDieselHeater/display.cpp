#include <SPI.h>
#include "AdaFruit_SH1106.h"
#include "protocol.h" 
#include "display.h"
#include "pins.h"
#include "BluetoothAbstract.h" 
#include "OLEDconsts.h"
#include "BTCWifi.h"

#define X_FANICON     55 
#define Y_FANICON     39
#define X_FUELICON    81 
#define Y_FUELICON    39
#define X_TARGETICON  31
#define Y_TARGETICON  39
#define Y_BASELINE    58
#define X_BATTICON    95
#define Y_BATTICON     0
#define X_GLOWICON    97
#define Y_GLOWICON    38
#define X_BODYBULB   119
#define X_BULB         1  // >= 1
#define Y_BULB         4
#define X_WIFIICON    22
#define Y_WIFIICON     0
#define X_BTICON      12
#define Y_BTICON       0


#define MINI_BATTLABEL
#define MINI_TEMPLABEL
#define MINI_TARGETLABEL
#define MINI_FANLABEL
#define MINI_GLOWLABEL
#define MINI_FUELLABEL
#define MINI_BODYLABEL

//
// **** NOTE: There are two very lame libaries conspiring to make life difficult ****
// A/ The ESP32 SPI.cpp library instatiates an instance of SPI, using the VSPI port (and pins)
// B/ The Adfruit_SH1106 library has hard coded "SPI" as the SPI port instance
//
// As an ESP32 has a pin multiplexer, this is very bad form.
// The design uses the defacto HSPI pins (and port), 
// You **MUST comment out the SPIClass SPI(VSPI);**  at the end of the ESP32 SPI library
// then we declare "SPI" here, which will use HSPI!!!!

// 128 x 64 OLED support
SPIClass SPI;    // default constructor opens HSPI on standard pins : MOSI=13,CLK=14,MISO=12(unused)
Adafruit_SH1106 display(OLED_DC_pin,  -1, OLED_CS_pin);

bool animatePump = false;
bool animateRPM = false;
bool animateGlow = false;


extern float fFilteredTemperature;
extern CBluetoothAbstract& getBluetoothClient();

void showThermometer(float desired, float actual);
void showBodyThermometer(int actual);
void showBTicon();
void showWifiIcon();
void showBatteryIcon(float voltage);
void showGlowPlug(int power);
void showFan(int RPM);
void showFuel(float rate);
void showRunState(int state, int errstate);
void printMiniNumericString(int xPos, int yPos, const char* str);
void printRightJustify(const char* str, int yPos, int RHS=128);

void initOLED()
{
  SPI.setFrequency(8000000);
  // SH1106_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SH1106_SWITCHCAPVCC, 0, false);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
}

void updateOLED(const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  int runstate = HtrFrame.getRunState(); 
  int errstate = HtrFrame.getErrState(); 
  if(errstate) errstate--;  // correct for +1 biased return value
  
  display.clearDisplay();

  float desiredT = 0;
  if(runstate && (runstate <= 5)) {
    if(CtlFrame.isThermostat())
      desiredT = CtlFrame.getTemperature_Desired();
    else
      desiredT = -HtrFrame.getPump_Fixed() * 0.1f;
  }

  showThermometer(desiredT,    // read values from most recently sent [BTC] frame
                  fFilteredTemperature);

  if(getBluetoothClient().isConnected())
    showBTicon();

  if(isWifiConnected()) {
    showWifiIcon();
    display.fillRect(X_WIFIICON + 8, Y_WIFIICON + 5, 10, 7, BLACK);
    printMiniNumericString(X_WIFIICON + 9, Y_WIFIICON + 6, "AP");
  }

  float voltage = HtrFrame.getVoltage_Supply() * 0.1f;
  showBatteryIcon(voltage);

  animateRPM = false;
  animatePump = false;
  animateGlow = false;

  if(runstate) {
    float power = HtrFrame.getGlowPlug_Current() * 0.01 * HtrFrame.getGlowPlug_Voltage() * 0.1;
    if(power > 1) {
      showGlowPlug(int(power));
    }

    showFan(HtrFrame.getFan_Actual());

    showFuel(HtrFrame.getPump_Actual() * 0.1f);

    showBodyThermometer(HtrFrame.getTemperature_HeatExchg());
  }
  else {
    if(isWifiConnected()) {
      printRightJustify(getWifiAddrStr(), 57);
    }
  }

  showRunState(runstate, errstate);

}


void animateOLED()
{
  static int fan = 0;
  static int drip = 0;
  static int heat = 0;

  if(animatePump || animateRPM || animateGlow) {

    if(animatePump) {
      // erase region of fuel icon
      display.fillRect(X_FUELICON, Y_FUELICON, 7, 16, BLACK);
      display.drawBitmap(X_FUELICON, Y_FUELICON+(drip/2), FuelIcon, 7, 12, WHITE);
      drip++;
      drip &= 0x07;
    }

    if(animateRPM) {
      // erase region of fuel icon
      display.fillRect(X_FANICON, Y_FANICON, 16, 16, BLACK);
      switch(fan) {
        case 0: display.drawBitmap(X_FANICON, Y_FANICON, FanIcon1, 16, 16, WHITE); break;
        case 1: display.drawBitmap(X_FANICON, Y_FANICON, FanIcon2, 16, 16, WHITE); break;
        case 2: display.drawBitmap(X_FANICON, Y_FANICON, FanIcon3, 16, 16, WHITE); break;
        case 3: display.drawBitmap(X_FANICON, Y_FANICON, FanIcon4, 16, 16, WHITE); break;
      }
      fan++;
      fan &= 0x03;
    }
    
    if(animateGlow) {
      display.fillRect(X_GLOWICON, Y_GLOWICON, 17, 10, BLACK);
      display.drawBitmap(X_GLOWICON, Y_GLOWICON, GlowPlugIcon, 16, 9, WHITE);
      display.drawBitmap(X_GLOWICON, Y_GLOWICON + 2 + heat, GlowHeatIcon, 17, 2, WHITE);
      heat -= 2;
      heat &= 0x07;
    }
  }
  display.display();
}


#define TEMP_YPOS(A) ((((20 - A) * 3) / 2) + 22)
void showThermometer(float desired, float actual) 
{
  char msg[16];
  display.clearDisplay();
  // draw bulb design
  display.drawBitmap(X_BULB, Y_BULB, thermometerBitmap, 8, 50, WHITE);
  // draw mercury
  int yPos = Y_BULB + TEMP_YPOS(actual);
  display.drawLine(X_BULB + 3, yPos, X_BULB + 3, Y_BULB + 42, WHITE);
  display.drawLine(X_BULB + 4, yPos, X_BULB + 4, Y_BULB + 42, WHITE);  
  // print actual temperature
#ifdef MINI_TEMPLABEL  
  sprintf(msg, "%.1f`C", actual);
  printMiniNumericString(0, Y_BASELINE, msg);
#else
  display.setTextColor(WHITE);
  display.setCursor(0, Y_BASELINE);
  display.print(actual, 1);
#endif

  // draw set point
  if(desired) {
    display.drawBitmap(X_TARGETICON, Y_TARGETICON, TargetIcon, 13, 13, WHITE);   // set indicator against bulb
    char msg[16];
    if(desired > 0) {
      int yPos = Y_BULB + TEMP_YPOS(desired) - 2;
      display.drawBitmap(X_BULB-1, yPos, thermoPtr, 3, 5, WHITE);   // set indicator against bulb
      sprintf(msg, "%.0f`C", desired);
    }
    else {
      sprintf(msg, "%.1fHz", -desired);
    }
#ifdef MINI_TARGETLABEL
    int xPos = X_TARGETICON + 7 - strlen(msg) * 2;    // 2 = 1/2 width mini font
    printMiniNumericString(xPos, Y_BASELINE, msg);
#else
    int xPos = X_TARGETICON + 6 - strlen(msg) * 3;    // 3 = 1/2 width normal font
    display.setCursor(xPos, Y_BASELINE);
    display.print(msg);
#endif
  }
}

#define BODY_YPOS(A) ((((100 - A) * 3) / 16) + 22)   // 100degC centre - ticks +- 80C
void showBodyThermometer(int actual) 
{
  // draw bulb design
  display.drawBitmap(X_BODYBULB, Y_BULB, thermometerBitmap, 8, 50, WHITE);
  // draw mercury
  int yPos = Y_BULB + BODY_YPOS(actual);
  display.drawLine(X_BODYBULB + 3, yPos, X_BODYBULB + 3, Y_BULB + 42, WHITE);
  display.drawLine(X_BODYBULB + 4, yPos, X_BODYBULB + 4, Y_BULB + 42, WHITE);
  // print actual temperature
  display.setTextColor(WHITE);
  char label[16];
  // determine width and position right justified
#ifdef MINI_BODYLABEL
  sprintf(label, "%d`C", actual);
  int width = strlen(label) * 4;
  printMiniNumericString(127-width, Y_BASELINE, label);
#else
  sprintf(label, "%d", actual);
  int width = strlen(label) * 6;
  display.setCursor(127-width, Y_BASELINE);
  display.print(label);
#endif
}

void showBTicon()
{
  display.drawBitmap(X_BTICON, Y_BTICON, BTicon, W_BTICON, H_BTICON, WHITE);
}

void showWifiIcon()
{
  display.drawBitmap(X_WIFIICON, Y_WIFIICON, wifiIcon, W_WIFIICON, H_WIFIICON, WHITE);
}

void showBatteryIcon(float voltage)
{
  display.drawBitmap(X_BATTICON, Y_BATTICON, BatteryIcon, 15, 10, WHITE);
#ifdef MINI_BATTLABEL
  char msg[16];
  sprintf(msg, "%.1fV", voltage);
  int xPos = X_BATTICON + 7 - strlen(msg) * 2;
  printMiniNumericString(xPos, Y_BATTICON+12, msg);
#else
  display.setCursor(85, 12);
  display.setTextColor(WHITE);
  display.print(voltage, 1);
  display.print("V");
#endif

  // nominal 10.5 -> 13.5V bargraph
//  int Capacity = int((voltage - 11.0) * 4.5);
//  int Capacity = (voltage - 11.4) * 7;
  int Capacity = (voltage - 10.7) * 4;
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  display.fillRect(X_BATTICON+2 + Capacity, Y_BATTICON+2, 11-Capacity, 6, BLACK);
}

void showGlowPlug(int power)
{
  display.drawBitmap(X_GLOWICON, Y_GLOWICON, GlowPlugIcon, 16, 9, WHITE);
//  animateGlow = true;
#ifdef MINI_GLOWLABEL  
  char msg[16];
  sprintf(msg, "%dW", power);
  int xPos = X_GLOWICON + 9 - strlen(msg) * 2;
  printMiniNumericString(xPos, Y_GLOWICON+12, msg);
#else
  display.setCursor(X_GLOWICON, Y_GLOWICON+12);
  display.print(power);
  display.print("W");
#endif
}

void showFan(int RPM)
{
  // NOTE: fan rotation animation performed in animateOLED
  animateRPM = RPM != 0;   // used by animation routine

  display.setTextColor(WHITE);
  char msg[16];
  sprintf(msg, "%d", RPM);
#ifdef MINI_FANLABEL  
  int xPos = X_FANICON + 8 - strlen(msg) * 2;    // 3 = 1/2 width font
  printMiniNumericString(xPos, Y_BASELINE, msg);
#else
  int xPos = X_FANICON + 8 - ( strlen(msg) * 3);    // 3 = 1/2 width font
  display.setCursor(xPos, Y_BASELINE);
  display.print(msg);
#endif
}

void showFuel(float rate)
{
  // NOTE: fuel drop animation performed in animateOLED
  animatePump = rate != 0;    // used by animation routine
  if(rate) {
    char msg[16];
    sprintf(msg, "%.1f", rate);
#ifdef MINI_FUELLABEL
    int xPos = X_FUELICON + 3 - strlen(msg) * 2;    // 3 = 1/2 width font
    printMiniNumericString(xPos, Y_BASELINE, msg);
#else
    int xPos = X_FUELICON + 3 - ( strlen(msg) * 3);    // 3 = 1/2 width font
    display.setCursor(xPos, Y_BASELINE);
    display.setTextColor(WHITE);
    display.print(msg);
#endif
  }
}

void showRunState(int runstate, int errstate) 
{
  static bool toggle = false;
  const char* toPrint = NULL;
  int yPos = 25;
  display.setTextColor(WHITE, BLACK);
  if(runstate >= 0 && runstate <= 8) {
    if(((runstate == 0) || (runstate > 5)) && errstate) {
      // create an "E-XX" message to display
      char msg[16];
      sprintf(msg, "E-%02d", errstate);
      int xPos = 64 - ((strlen(msg)/2) * 6);
      if(runstate > 5)
        yPos -= 8;
      display.setCursor(xPos, yPos);
      yPos += 8;
      // flash error code
      toggle = !toggle;
      if(toggle)
        display.print(msg);
      else {
        display.print("     ");
      }
      // bounds limit error and gather message
      if(errstate > 10) errstate = 11;
      toPrint = Errstates[errstate-1];
    }
    else {
      toPrint = Runstates[runstate];
    }
  }
  if(toPrint) {
      int width = strlen(toPrint);
      int xPos = 64 - ((width/2) * 6);
      display.setCursor(xPos, yPos);
      display.print(toPrint);
  }
}


void printMiniNumericString(int xPos, int yPos, const char* str) 
{
//  xPos = display.getCursorX();
//  yPos = display.getCursorY();
  const char* pNext = str;
  while(*pNext) {
    const uint8_t* pBmp = NULL;
    switch(*pNext) {
      case '0':  pBmp = Mini0; break;
      case '1':  pBmp = Mini1; break;
      case '2':  pBmp = Mini2; break;
      case '3':  pBmp = Mini3; break;
      case '4':  pBmp = Mini4; break;
      case '5':  pBmp = Mini5; break;
      case '6':  pBmp = Mini6; break;
      case '7':  pBmp = Mini7; break;
      case '8':  pBmp = Mini8; break;
      case '9':  pBmp = Mini9; break;
      case ' ':  pBmp = MiniSpc; break;
      case '.':  pBmp = MiniDP; break;
      case '`':  pBmp = MiniDeg; break;
      case 'A':  pBmp = MiniA; break;
      case 'C':  pBmp = MiniC; break;
      case 'H':  pBmp = MiniH; break;
      case 'P':  pBmp = MiniP; break;
      case 'V':  pBmp = MiniV; break;
      case 'W':  pBmp = MiniW; break;
      case 'z':  pBmp = Miniz; break;
    }
    if(pBmp) {
      display.drawBitmap(xPos, yPos, pBmp, 3, 5, WHITE);
    }
    xPos += 4;
    pNext++;
  }
}

void printRightJustify(const char* str, int yPos, int RHS)
{
  int xPos = RHS - strlen(str) * 6;
  display.setCursor(xPos, yPos);
  display.print(str);
}

void printMiniRightJustify(const char* str, int yPos, int RHS)
{
  int xPos = RHS - strlen(str) * 4;
  display.setCursor(xPos, yPos);
//  display.print(str);
}
