#ifndef MISC_H
#define MISC_H

//STL
#include <utility>
#include <vector>
#include <string>

//OpenCV Headers
#ifdef __APPLE__
#  include <OpenCV/OpenCV.h>

//Min/max are missing from STL utility (wtf mac?)
namespace std
{
    template<typename T> T min(T& a, T& b) { return (a < b) ? a : b; }
    template<typename T> T max(T& a, T& b) { return (a > b) ? a : b; }
}

#else
#  include <opencv/cv.h>
#  include <opencv/highgui.h>
#endif


//Retrieves temporary directory path
extern char * getTempDirectory();

//Saves a PLY file
extern void savePly(
    const std::string& filename, 
    const std::vector<vec3>& points, 
    const std::vector<ivec3>& colors);

#endif
