Copy libdriver.a to arduino-esp32 lib folder

eg for PIO, 
C:\Users\$USER$\.platformio\packages\framework-arduinoespressif32\tools\sdk\lib

ensure the arduinopackage gets rebuilt.
eg for PIO
delete Afterburner\.pio\build\esp32dev\libFrameworkArduino.a

Next time you rebuild, the Arduino package will include the patched libdriver.a
which solves the problem caused by the error message being printed

