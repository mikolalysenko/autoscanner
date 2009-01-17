//This module includes system specific functions which require
//non-portable library calls to get working (ex. finding /tmp/)
#ifndef SYSTEM_H
#define SYSTEM_H

//Retrieves temporary directory
extern char * getTempDirectory();

#endif
