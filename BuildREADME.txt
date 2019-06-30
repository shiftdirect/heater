This directory structure includes an Arduino path, and a src path.

The Arduino path is only to support builing with the restrictive Arduino IDE.
The src path is for the awesome PlatformIO under VScode. 
Use it, you will not regret it.

Arduino builds are sloooooowwwww. 
Do it that way if you must, but seriously don't!
PlatformIO rocks - just load the root repo directory with PIO in VScode 
and away you go.

The Arduino\Afterburner\Afterburner.ino file is a hard symbolic link to the
proper src\Afterburner\Afterburner.cpp file.

Likewise the Arduino\Afterburner\src and Arduino\Afterburner\data paths are 
hard junctions to the src\Afterburner\src and src\Afterburner\data files.  