AfterburnerCRC.cpp is used to append a CRC-16 value to the 
compiled binary file.

This CRC is used to confirm the binary image uploaded to the
Afterburner was genuinely intended for the Afterburner.

Naively attempting to upload the direct compiled output binary
will result in rejection of the upload attempt.

You will need to compile Afterburner.cpp and copy the resultant 
executable to the repository root (adjacent to plaformio.ini)  