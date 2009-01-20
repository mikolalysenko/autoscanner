#include <iostream>
#include <string>
#include <cstdlib>

#include "system.h"

using namespace std;

//Locates the temp directory
string getTempDirectory()
{
    if(getenv("TMPDIR"))
        return getenv("TMPDIR");
    if(getenv("TEMP"))
        return getenv("TEMP");
    if(getenv("TMP"))
        return getenv("TMP");
    return "/tmp/";
}

//Pixel ostream
ostream& operator<<(ostream& os, const Color & pix)
{
    return os << "[r=" << pix.r << ",g=" << pix.g << ",b=" << pix.g << "]";
}
