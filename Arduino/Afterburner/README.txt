
TO WORK WITH ARDUINO IDE, EVERYTHING IN THIS FOLDER IS FAKE!

When you pull from gitlab, no symbolic links will be created.

You need to execute MakeSymLinks_Windows.bat from Explorer.

Arduino insists upon .ino for their projects, and the .ino
file name also has to match the parent directory name.
 
The BIG trick here is Afterburner.ino is empty - zilch, nada, nothing!
All the REAL source code lives via the src symbolic link.
The real core exists as a .cpp file: repo\src\Afterburner.cpp

Arduino\Afterburner\Afterburner.ino is EMPTY
Arduino\Afterburner\src links to repo\src\  
Arduino\Afterburner\data links to repo\data

Ugggh. I hate Arduino IDE (and it's build environment!)
