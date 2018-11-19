#include <stdint.h>

#define NEW_LAYOUT

#ifdef NEW_LAYOUT
const uint8_t LED_Pin = 2;
const uint8_t HC05_KeyPin = 4;
const uint8_t TxEnbPin = 5;
const uint8_t OLED_MISO_pin = 12;    // HSPI std pins
const uint8_t OLED_MOSI_pin = 13;    //  "
const uint8_t OLED_CLK_pin = 14;     //  "
const uint8_t DS18B20_Pin = 15; 
const uint8_t Rx1Pin = 16;
const uint8_t Tx1Pin = 17;
const uint8_t Rx2Pin = 18;
const uint8_t Tx2Pin = 19;
const uint8_t OLED_SDA = 21;         // I2C std pins
const uint8_t OLED_SCK = 22;         //  "
const uint8_t HC05_SensePin = 23;
const uint8_t OLED_DC_pin = 26;
const uint8_t OLED_CS_pin = 27;

// uncommitted
const uint8_t pin32 = 32;            // GPIO
const uint8_t pin33 = 33;            // GPIO
const uint8_t pin34 = 34;            // input only, no chip pullup
const uint8_t pin35 = 35;            // input only, no chip pullup 

const uint8_t ListenOnlyPin = 36;    // input only, no chip pullup
const uint8_t WiFi_TriggerPin = 39;  // input only, no chip pullup

#else
// OLD pin outs - OBSOLETE

const uint8_t LED_Pin = 2;
const uint8_t HC05_SensePin = 4;
const uint8_t DS18B20_Pin = 5;
const uint8_t OLED_MISO_pin = 12;   // HSPI std pins
const uint8_t OLED_MOSI_pin = 13;   //  "
const uint8_t OLED_CLK_pin = 14;    //  "
const uint8_t HC05_KeyPin = 15;
const uint8_t Tx2Pin = 16;
const uint8_t Rx2Pin = 17;
const uint8_t Tx1Pin = 18;
const uint8_t Rx1Pin = 19;
const uint8_t OLED_SDA = 21;        // I2C std pins
const uint8_t OLED_SCK = 22;        //  "
const uint8_t TxEnbPin = 22;
const uint8_t OLED_DC_pin = 26;
const uint8_t OLED_CS_pin = 27;

const uint8_t ListenOnlyPin = 36;    
const uint8_t WiFi_TriggerPin = 39;  

#endif
