//This module includes system specific functions which require
//non-portable library calls to get working (ex. finding /tmp/)
#ifndef SYSTEM_H
#define SYSTEM_H

//8-bit sizes
typedef char                    byte;
typedef unsigned char           ubyte;

//Clamp/saturate color components
template<typename T> T clamp(T a, T l, T h)     { return min(max(a, l), h); }
template<typename T> T saturate(T a)            { return clamp(a, 0, 1); }

//Retrieves temporary directory
extern char * getTempDirectory();

#endif
