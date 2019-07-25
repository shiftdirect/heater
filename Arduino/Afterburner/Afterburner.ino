/**********************************************************************
 * This file is deliberately empty.
 * 
 * It only exists to satisfy the Arduino IDE's perverse requirement that a
 *  .ino must live directly below a parent directory, with the same name.
 *   
 * In this instance Afterburner\Afterburner.ino
 * 
 * I seriously recommend you use PLatformIO with your favourite editor.
 * 
 * The real source code for the entire project is linked to via a symbolic 
 * link to the ClonedRepo\src, lib & data directories.
 * 
 * ie ClonedRepo\Arduino\Afterburner\src\src -> ClonedRepo\src  (..\..\src)
 * ie ClonedRepo\Arduino\Afterburner\src\lib -> ClonedRepo\lib  (..\..\lib)
 * ie ClonedRepo\Arduino\Afterburner\data -> ClonedRepo\data  (..\..\data)
 * 
 * A batch file is in this folder to create these links, please use it first.
 * 
 * Whilst initially alarming that is .ino file is empty, the Arduino IDE 
 * happily creates the required executable :-)
 * 
 * The REAL host of setup() and loop() resides in ClonedRepo\src\Afterburner.cpp
 * 
 *
 *****************************************************************************
 *****************************************************************************
 **                                                                         **
 **  DUMP the Arduino IDE, and use PlatformIO.                              **
 **  Load the ClonedRepo path into PlatformIO.                              ** 
 **  Builds much faster and meshes well with decent programming editors :-) **
 **                                                                         **
 *****************************************************************************
 *****************************************************************************/