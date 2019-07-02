
EVERYTHING IN THIS FOLDER IS FAKE!

When you pull from gitlab, no symbolic links will be created.

Then execute MakeSymLinks_Windows.bat from Explorer.

The real root source exists as a .cpp file: repo\src\Afterburner\Afterburner.cpp
However Arduino insists upon .ino for their projects, and the file name also 
has to match the parent directory name. 

If .ino and .cpp live in same directory - double trouble at link time!
Works for PlatformIO, but not Arduino builds.

Ugggh. I hate Arduino IDE (and it's build environment!)

Likewise:
 src is linked to repo\src\Afterburner\src
 data is linked to repo\src\Afterburner\data