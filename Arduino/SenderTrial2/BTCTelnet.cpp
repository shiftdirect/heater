// Hopefully this code will be removed on the next commit as I dont think its required
//#define TELNET

#ifdef TELNET
#define DebugPort Debug
#endif

#ifndef TELNET
#define DebugPort DebugPort
#endif

#include "BTCTelnet.h"
#include "debugport.h"

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void initTelnet() {
  server.begin();
  server.setNoDelay(true);

  DebugPort.print("Ready! Use 'telnet ");
  DebugPort.print(WiFi.localIP());
  DebugPort.println(" 23' to connect");
}

void doTelnet() {
  uint8_t i;
    //check if there are any new clients
    if (server.hasClient()){
      for(i = 0; i < MAX_SRV_CLIENTS; i++){
        //find free/disconnected spot
        if (!serverClients[i] || !serverClients[i].connected()){
          if(serverClients[i]) serverClients[i].stop();
          serverClients[i] = server.available();
          if (!serverClients[i]) DebugPort.println("available broken");
          DebugPort.print("New client: ");
          DebugPort.print(i); DebugPort.print(' ');
          DebugPort.println(serverClients[i].remoteIP());
          break;
        }
      }
      if (i >= MAX_SRV_CLIENTS) {
        //no free/disconnected spot so reject
        server.available().stop();
      }
    }
    //check clients for data
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        if(serverClients[i].available()){
          //get data from the telnet client and push it to the UART
          while(serverClients[i].available()) DebugPort.write(serverClients[i].read());
        }
      }
      
      else {
        if (serverClients[i]) {
          serverClients[i].stop();
        }
      }
  }
    
    
    //check UART for data
    if(DebugPort.available()){
      size_t len = DebugPort.available();
      uint8_t sbuf[len];
      DebugPort.readBytes(sbuf, len);
      //push UART data to all connected telnet clients
      for(i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()){
          serverClients[i].write(sbuf, len);
          delay(1);
        }
      }
    }
  else {
    DebugPort.println("WiFi not connected!");
    for(i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i]) serverClients[i].stop();
    }
    delay(1000);
    }
}

