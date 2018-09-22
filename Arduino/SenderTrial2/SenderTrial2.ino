 /*
  Chinese Heater Half Duplex Serial Data Send Test Tool

  Connects to the blue wire of a Chinese heater, which is the half duplex serial link.
  Receives data from serial port 1. 
  
  Terminology: Tx is to the heater unit, Rx is from the heater unit.
  
  The binary data is received from the line.
  If it has been > 100ms since the last activity this indicates a new frame 
  sequence is starting, synchronise as such then count off the next 48 bytes 
  storing them in the Data array. 

  The "outer loop" then detects the count of 48 and packages the data to be sent 
  over Serial to the USB attached PC.

  Typical data frame timing on the blue wire is:
  __Tx_Rx____________________________Tx_Rx____________________________Tx_Rx___________
    
  Rx to next Tx delay is always > 100ms and is paced by the controller.
  The delay before seeing Rx data after Tx is usually much less than 10ms.
  **The heater only ever sends Rx data in response to a data frame from the controller**
  
  Resultant data is tagged and sent out on serial port 0 (the default debug port), 
  along with a timestamp for relative timing.
  
  This example works only with boards with more than one serial port like Arduino 
  Mega, Due, Zero etc.

  The circuit:
  - Blue wire connected to Serial 1 Rx input - preferably with series 680ohm resistor.
  - Serial logging software on Serial port 0 via USB link

  created 24 Aug 2018 by Ray Jones

  modified 25 Aug by Ray Jones
    - simplifed to read 48 bytes, synchronised by observing a long pause 
      between characters. The heater only sends when prompted.
      No longer need to discrimate which packet of data would be present.

  This example code is in the public domain.
*/

#include "CFrame.h"

CFrame Controller(CFrame::TxMode);
CFrame TxFrame(CFrame::TxMode);
CFrame Heater1;
CFrame Heater2;
const int TxEnbPin = 17;
long lastRxTime;        // used to calculate inter character delay
long TxEnbTime;       // used to reset TxEnb low
bool bOnEvent = false;
bool bOffEvent = false;

void CheckTx();


void setup() 
{
  pinMode(TxEnbPin, OUTPUT);
  digitalWrite(TxEnbPin, LOW);
  // initialize listening serial port
  // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
  // Tx/Rx data to/from heater, special baud rate for Chinese heater controllers
  Serial1.begin(25000);   
  pinMode(19, INPUT_PULLUP);
  
  // initialise serial monitor on serial port 0
  Serial.begin(115200);
  
  // prepare for first long delay detection
  lastRxTime = millis();

}

void loop() 
{
  static int count = 0;
  static unsigned long lastTx = 0;
  static bool bAllowTxSlot = false;
  static int Stage = -1;
  
  char str[16];
  
  unsigned long timenow = millis();

  if(Serial.available()) {
    char rxval = Serial.read();
    if(rxval  == '+') {
      bOnEvent = true;
    }
    if(rxval  == '-') {
      bOffEvent = true;
    }
  }

  if((Stage == 4) && (timenow - lastTx) > 10) {
    TxEnbTime = timenow;
    if(TxEnbTime == 0)
      TxEnbTime++;
    digitalWrite(TxEnbPin, HIGH);
  }

  CheckTx();
  // check serial data has gone quite for a while so we can trample in...
  // calc elapsed time since last rxd byte to detect start of frame sequence
 /* unsigned long TxSlot = timenow - lastRxTime;
    
//  if(Slot > 50 && TxEnbTime == 0 && (bOnEvent || bOffEvent)) {
  if(bAllowTxSlot && (TxSlot > 50) && (TxEnbTime == 0)) {
    TxEnbTime = timenow;
    if(TxEnbTime == 0)
      TxEnbTime++;
    digitalWrite(TxEnbPin, HIGH);
  }
*/
  // read from port 1, the "Tx Data" (to heater), send to the serial monitor:
  if (Serial1.available()) {
  
    // calc elapsed time since last rxd byte to detect start of frame sequence
    unsigned long diff = timenow - lastRxTime;
    lastRxTime = timenow;
    
    if((Stage == -1) && (diff > 100)) {       // this indicates the start of a new frame sequence from the controller
      Stage = 0;
      count = 0;
    }
    
    int inByte = Serial1.read(); // read hex byte

    if(Stage == 0) {
      Controller.Data[count++] = inByte;
      if(count == 24) {
        Stage = 1;
      }
    }

    if(Stage == 2) {
      Heater1.Data[count++] = inByte;
      if(count == 24) {
        Stage = 3;
      }
    }

    if(Stage == 6) {
      Heater2.Data[count++] = inByte;
      if(count == 24) {
        Stage = 7;
      }
    }

  }

  // dump to PC after capturing all 24 Rx bytes in a frame session
  if(Stage == 1) {  // filled controller frame, dump
  
    char str[16];
    sprintf(str, "Ctrl  ", lastRxTime);
    Serial.print(str);                 // print timestamp
    for(int i=0; i<24; i++) {
    
      sprintf(str, "%02X ", Controller.Data[i]);  // make 2 dig hex values
      Serial.print(str);               // and print     
                          
    }
//    Serial.println();                  // newline and done

    Stage = 2;
    count = 0;
    
  }  // Stage == 1

  if(Stage == 3) {  // filled heater frame, dump
  
    char str[16];
    sprintf(str, "  Htr1  ", lastRxTime);
    Serial.print(str);                 // print timestamp
    for(int i=0; i<24; i++) {
    
      sprintf(str, "%02X ", Heater1.Data[i]);  // make 2 dig hex values
      Serial.print(str);               // and print     
                          
    }
    Serial.println();                  // newline and done

    Stage = 4;
    count = 0;
    lastTx = timenow;
    
  }  // Stage == 1

  if(Stage == 7) {  // filled heater frame, dump
  
    char str[16];
    sprintf(str, "  Htr2  ", lastRxTime);
    Serial.print(str);                 // print timestamp
    for(int i=0; i<24; i++) {
    
      sprintf(str, "%02X ", Heater2.Data[i]);  // make 2 dig hex values
      Serial.print(str);               // and print     
                          
    }
    Serial.println();                  // newline and done

    Stage = -1;
    count = 0;
    
  }  // Stage == 1

}  // loop
 
void CheckTx()
{
  char str[16];
  
  if(TxEnbTime) {
    long diff = timenow - TxEnbTime;
    if(diff >= 12) {
      TxEnbTime = 0;
      digitalWrite(TxEnbPin, LOW);
      Stage = 6;
    }
  }
  
  if((Stage == 4) && (timenow - lastTx) > 10) {

    Stage = 5;

    TxFrame.Tx.Byte0 = 0x78;
    TxFrame.setTemperature_Desired(35);
    TxFrame.setTemperature_Actual(22);
    TxFrame.Tx.OperatingVoltage = 240;
    TxFrame.setPump_Min(16);
    TxFrame.setPump_Max(55);
    TxFrame.setFan_Min(1680);
    TxFrame.setFan_Max(4500);

    if(bOnEvent) {
      bOnEvent = false;
      TxFrame.Tx.Command = 0xa0;
    }
    else if(bOffEvent) {
      bOffEvent = false;
      TxFrame.Tx.Command = 0x05;
    }
    else {
      TxFrame.Tx.Command = 0x00;
    }
    
    TxFrame.setCRC();
    
    // send to serial monitor using ASCII
    Serial.print("Us    ");                       // and print ASCII data    
    for(int i=0; i<24; i++) {
      sprintf(str, "%02X ", TxFrame.Data[i]);  // make 2 dig hex ASCII values
      Serial.print(str);                       // and print ASCII data    
    }

    // send to heater - using binary 
    digitalWrite(TxEnbPin, HIGH);
    for(int i=0; i<24; i++) {
      Serial1.write(TxFrame.Data[i]);                  // write native binary values
    }
  }

}