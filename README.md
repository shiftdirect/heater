# BluetoothHeater

Bluetooth interface for Chinese Diesel Heaters

Working so far:
* Power On/Off
* Temperature + & -
* Fuel mixture tuning
* Temperature sensing (DS18B20)
* Heat exchanger body temperature as reported by heater unit
* Interface to external SH1106 based OLED for a full featured controller
* 5 button keypad interface
* Smart error detection, observes unusual heater state progression (ignition fail)
* Bluetooth Connectivity
* Bluetooth Control App for Android (Alpha Testing)
* WiFi Connection to existing network or Standalone Access Point Mode (Passwd: thereisnospoon)
* list to be expanded

To be implemented 
--------------------------
* Wifi control
* DebugPort data to be sent via Telnet if/when available on the network.
* MQTT pub/sub 
* Expand hardware compatability with different MCU setups.  IE Arduino Due/Mega/Pro ESP8266 & ESP32
* Documentation
