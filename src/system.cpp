#include <iostream>
#include <string>
#include <fstream>

#include <Eigen/Core>

USING_PART_OF_NAMESPACE_EIGEN

using namespace std;

//Locates the temp directory
char * getTempDirectory()
{
    if(getenv("TMPDIR"))
        return getenv("TMPDIR");
    if(getenv("TEMP"))
        return getenv("TEMP");
    if(getenv("TMP"))
        return getenv("TMP");
    return "/tmp/";
}

