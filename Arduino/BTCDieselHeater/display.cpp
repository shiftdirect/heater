#include <SPI.h>
#include "AdaFruit_SH1106.h"
#include "protocol.h" 
#include "display.h"
#include "pins.h"
#include "BluetoothAbstract.h" 
#include "OLEDconsts.h"

#define X_FANICON 60 //46
#define Y_FANICON 39
#define X_FUELICON 90 // 80
#define Y_FUELICON 39
#define X_TARGETICON 30
#define Y_TARGETICON 39
#define Y_BASELINE 57

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
SPIClass SPI;    // default constructor opens HSPI on pins
Adafruit_SH1106 display(LCDpin_DC,  -1, LCDpin_CS);

int prevPump;
int prevRPM;


extern float fFilteredTemperature;
extern CBluetoothAbstract& getBluetoothClient();

void showThermometer(float desired, float actual);
void showBodyThermometer(int actual);
void showBTicon();
void showBatteryIcon(float voltage);
void showGlowPlug(int power);
void showFan(int RPM);
void showFuel(float rate);
void showRunState(int state, int errstate);
void printMiniNumericString(int xPos, int yPos, const char* str);

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
  if(runstate) {
    if(CtlFrame.isThermostat())
      desiredT = CtlFrame.getTemperature_Desired();
    else
      desiredT = -HtrFrame.getPump_Fixed() * 0.1f;
  }

  showThermometer(desiredT,    // read values from most recently sent [BTC] frame
                  fFilteredTemperature);

  if(getBluetoothClient().isConnected())
    showBTicon();

  float voltage = HtrFrame.getVoltage_Supply() * 0.1f;
  showBatteryIcon(voltage);

  if(runstate) {
    float power = HtrFrame.getGlowPlug_Current() * 0.01 * HtrFrame.getGlowPlug_Voltage() * 0.1;
    if(power > 1)
      showGlowPlug(int(power));

    showFan(HtrFrame.getFan_Actual());

    showFuel(HtrFrame.getPump_Actual() * 0.1f);

    showBodyThermometer(HtrFrame.getTemperature_HeatExchg());
  }
  else {
    prevRPM = 0;
    prevPump = 0;
  }

  showRunState(runstate, errstate);

}


void animateOLED()
{
  static int fan = 0;
  static int drip = 0;

  if(prevPump || prevRPM) {

    if(prevPump) {
      // erase region of fuel icon
      display.fillRect(X_FUELICON, Y_FUELICON, 7, 16, BLACK);
      display.drawBitmap(X_FUELICON, Y_FUELICON+drip, FuelIcon, 7, 12, WHITE);
      drip++;
      drip &= 0x03;
    }

    if(prevRPM) {
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
  }
  display.display();
}


#define TEMP_YPOS(A) ((((20 - A) * 3) / 2) + 22)
#define BULB_X 1  // >= 1
#define BULB_Y 4
void showThermometer(float desired, float actual) 
{
  display.clearDisplay();
  // draw bulb design
  display.drawBitmap(BULB_X, BULB_Y, thermometerBitmap, 8, 50, WHITE);
  // draw mercury
  int yPos = BULB_Y + TEMP_YPOS(actual);
  display.drawLine(BULB_X + 3, yPos, BULB_X + 3, BULB_Y + 42, WHITE);
  display.drawLine(BULB_X + 4, yPos, BULB_X + 4, BULB_Y + 42, WHITE);
  // print actual temperature
  display.setTextColor(WHITE);
  display.setCursor(0, Y_BASELINE);
  display.print(actual, 1);
  // draw set point

  if(desired) {
    char msg[16];
    int desY = 19;
    display.drawBitmap(X_TARGETICON, Y_TARGETICON, TargetIcon, 13, 13, WHITE);   // set indicator against bulb
    desY += 14;
    if(desired > 0) {
      int yPos = BULB_Y + TEMP_YPOS(desired) - 2;
      display.drawBitmap(BULB_X-1, yPos, thermoPtr, 3, 5, WHITE);   // set indicator against bulb
      sprintf(msg, "%.0fC", desired);
    }
    else {
      sprintf(msg, "%.1fHz", -desired);
    }
    int xPos = X_TARGETICON + 6 - ( strlen(msg) * 3);    // 3 = 1/2 width font
    display.setCursor(xPos, Y_BASELINE);
    display.print(msg);
  }
}

#define BODYBULB_X 119
#define BODY_YPOS(A) ((((100 - A) * 3) / 16) + 22)   // 100degC centre - ticks +- 80C
void showBodyThermometer(int actual) 
{
  // draw bulb design
  display.drawBitmap(BODYBULB_X, BULB_Y, thermometerBitmap, 8, 50, WHITE);
  // draw mercury
  int yPos = BULB_Y + BODY_YPOS(actual);
  display.drawLine(BODYBULB_X + 3, yPos, BODYBULB_X + 3, BULB_Y + 42, WHITE);
  display.drawLine(BODYBULB_X + 4, yPos, BODYBULB_X + 4, BULB_Y + 42, WHITE);
  // print actual temperature
  display.setTextColor(WHITE);
  char label[16];
  sprintf(label, "%d", actual);
  // determine width and position right justified
  int width = strlen(label) * 6;
  display.setCursor(127-width, Y_BASELINE);
  display.print(label);
}

void showBTicon()
{
  display.drawBitmap(12, 0, BTicon, 6, 11, WHITE);
}

void showBatteryIcon(float voltage)
{
  display.drawBitmap(95, 0, BatteryIcon, 15, 10, WHITE);
  display.setCursor(85, 12);
  display.setTextColor(WHITE);
  display.print(voltage, 1);
  display.print("V");

  // nominal 10.5 -> 13.5V bargraph
  int Capacity = int((voltage - 11.0) * 4.5);
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  display.fillRect(97 + Capacity, 2, 11-Capacity, 6, BLACK);
}

#define XPOS_GLOW 35
#define YPOS_GLOW 0
void showGlowPlug(int power)
{
  display.drawBitmap(XPOS_GLOW, YPOS_GLOW, GlowPlugIcon, 16, 10, WHITE);
  display.setCursor(XPOS_GLOW, YPOS_GLOW+12);
  display.setTextColor(WHITE);
  display.print(power);
  display.print("W");
}

void showFan(int RPM)
{
  // NOTE: fan rotation animation performed in animateOLED
  prevRPM = RPM;   // used by animation routine

  display.setTextColor(WHITE);
  char msg[16];
  sprintf(msg, "%d", RPM);
  int xPos = X_FANICON + 8 - ( strlen(msg) * 3);    // 3 = 1/2 width font
  display.setCursor(xPos, Y_BASELINE);
  display.print(msg);
}

void showFuel(float rate)
{
  // NOTE: fuel drop animation performed in animateOLED
  prevPump = rate > 0;    // used by animation routine
  if(rate) {
    char msg[16];
    sprintf(msg, "%.1f", rate);
    int xPos = X_FUELICON + 3 - ( strlen(msg) * 3);    // 3 = 1/2 width font
    display.setCursor(xPos, Y_BASELINE);
    display.setTextColor(WHITE);
    display.print(rate, 1);
  }
}

void showRunState(int runstate, int errstate) 
{
  static bool toggle = false;
  const char* toPrint = NULL;
  int yPos = 25;
  display.setTextColor(WHITE, BLACK);
  if(runstate >= 0 && runstate <= 8) {
    if(runstate == 0 && errstate) {
      // create an "E-XX" message to display
      char msg[16];
      sprintf(msg, "E-%02d", errstate);
      int xPos = 64 - ((strlen(msg)/2) * 6);
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
      case '.':  pBmp = MiniDP; break;
      case '`':  pBmp = MiniDeg; break;
      case 'C':  pBmp = MiniC; break;
      case 'H':  pBmp = MiniH; break;
      case 'z':  pBmp = Miniz; break;
      case ' ':  pBmp = MiniSpc; break;
    }
    if(pBmp) {
      display.drawBitmap(xPos, yPos, pBmp, 3, 5, WHITE);
    }
    xPos += 4;
    pNext++;
  }
}