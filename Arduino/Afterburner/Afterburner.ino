/**********************************************************************
 * This file is deliberately empty
 * 
 * It only exists to satisfy the Arduino IDE's requirement that the
 *  .ino must live directly below a parent directory, with the same name
 *  
 * In this instance Afterburner\Afterburner.ino
 * 
 * The real source code for the entire project is linked to via a symbolic 
 * link to the ClonedRepo\src directory
 * 
 * ie ClonedRepo\Arduino\Afterburner\src -> ClonedRepo\src  (..\..\src)
 * 
 * Whilst initially alarming on first sight, the Arduino IDE happily 
 * creates the required executable :-)
 * 
 * The REAL host of setup() and loop() is ClonedRepo\src\Afterburner.cpp
 * 
 *
 *****************************************************************************
 *****************************************************************************
 **                                                                         **
 **  The reality is, DUMP the Arduino IDE, and use PlatformIO, loading      **  
 **  the ClonedRepo path.                                                   ** 
 **  Builds much faster and meshes well with decent programming editors :-) **
 **                                                                         **
 *****************************************************************************
 *****************************************************************************/