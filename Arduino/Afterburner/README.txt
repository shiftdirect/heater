
EVERYTHING IN THIS FOLDER IS FAKE!

They are symbolic links to the real locations in repo\src\Afterburner.
The directories must be soft links so thay can be added to git!

This is becasue Arduino insists upon .ino for their projects, and 
that file has to match the directory name. 

If .ino and .cpp live in same directory - double trouble at link time!

Ugggh.