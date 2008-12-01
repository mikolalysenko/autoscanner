#ifndef VIEW_H
#define VIEW_H

#include <vector>
#include "misc.h"

#define RES_R   3   
#define RES_G   3
#define RES_B   3

//A camera view
struct View
{
    //Image data
    IplImage * img;
    
    //Camera calibration data
    mat44 cam, cam_inv;
    
    //Camera center
    vec3 center;
    
    //Constructs a view
    View(IplImage *img_, mat44 K, mat44 R, mat44 S);
    
    //Reads
    vec3 readPixel(int ix, int iy) const;
    bool readFiltered(const vec3& point, vec3& color) const;
};


//Loads a view set from file (using Stanford format)
std::vector<View*>  loadViews(const char* filename, vec3 lo, vec3 hi, ivec3 box);

#endif
