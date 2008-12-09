#ifndef VIEW_H
#define VIEW_H

#include <cassert>
#include <vector>
#include <algorithm>
#include <string>
#include "misc.h"
#include "config.h"

using namespace cfg;

//A camera view
struct View
{
    //Image data
    IplImage * img;
    
    //Consistency data
    char * consist_data;
    
    //Camera calibration data
    mat44 cam, cam_inv;

    mat44 K, R, S;
    
    //Camera center
    vec3 center;
    
    //Constructs a view
    View(IplImage *img_, mat44 cam_, mat44 cam_inv_, vec3 center_);
    View(IplImage *img_, mat44 K_, mat44 R_, mat44 S_);
    View(config& cfg);
    
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
    
    config save(const std::string& name, const std::string& dir);
    void load(config& data);

private:
    void init(mat44 K_, mat44 R_, mat44 S_);
};

//Loads a view set from file (using Stanford format)
std::vector<View*>  loadViews(const char* filename, vec3 lo, vec3 hi, ivec3 box, float focal_length = 1);

//Saves a pile of views to an interchange format
void saveTempViews(const std::string& directory, const std::string& filename, std::vector<View*> views);
std::vector<View*> loadTempViews(const std::string& filename);

//Save cameras to PLY
void saveCameraPLY(const char * filename, std::vector<View*> views);

#endif
