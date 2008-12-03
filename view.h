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
    View(IplImage *img_, mat44 K, mat44 R, mat44 S);
    
    //Reads
    vec3 readPixel(int ix, int iy) const;
    
    //Reduce
    char consist(int ix, int iy) const
    {
        assert(ix >= 0 && ix < img->width && iy >= 0 && iy < img->height);
        return consist_data[ix + iy * img->width];
    }
    
    char& consist(int ix, int iy)
    {
        assert(ix >= 0 && ix < img->width && iy >= 0 && iy < img->height);
        return consist_data[ix + iy * img->width];
    }
    
    //Resets the consistency data
    void resetConsist() { std::fill(consist_data, consist_data + img->width * img->height, 0); }

    void writeConsist(const std::string& filename);
};

//Loads a view set from file (using Stanford format)
std::vector<View*>  loadViews(const char* filename, vec3 lo, vec3 hi, ivec3 box);

#endif
