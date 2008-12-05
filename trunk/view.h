#ifndef VIEW_H
#define VIEW_H

#include <cassert>
#include <vector>
#include <algorithm>
#include <string>
#include "misc.h"

//A camera view
struct View
{
    //Image data
    IplImage * img;
    
    //Consistency data
    char * consist_data;
    
    //Camera calibration data
    mat44 cam, cam_inv;
    
    //Camera center
    vec3 center;
    
    //Constructs a view
    View(IplImage *img_, mat44 cam_, mat44 cam_inv_, vec3 center_);
    View(IplImage *img_, mat44 K, mat44 R, mat44 S);
    
    //Reads
    vec3 readPixel(int ix, int iy) const;

    bool in_bounds(int ix, int iy) const {
        return ix >= 0 && ix < img->width && iy >= 0 && iy < img->height;
    }
    
    //Reduce
    char consist(int ix, int iy) const
    {
        return consist(ix, iy);
    }
    
    char& consist(int ix, int iy)
    {
        assert(in_bounds(ix, iy));
        return consist_data[ix + iy * img->width];
    }
    
    //Resets the consistency data
    void resetConsist() { std::fill(consist_data, consist_data + img->width * img->height, 0); }

    void writeConsist(const std::string& filename);
};

//Loads a view set from file (using Stanford format)
std::vector<View*>  loadViews(const char* filename, vec3 lo, vec3 hi, ivec3 box, float focal_length);

//Saves a pile of views to an interchange format
void saveTempViews(const char * directory, std::vector<View*> views);
std::vector<View*> loadTempViews(const char* directory);

#endif
