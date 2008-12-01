#ifndef VOLUME_H
#define VOLUME_H

#include <cassert>

#include "misc.h"


//Voxel data structure
struct Volume
{
    unsigned char * data;
    size_t xRes, yRes, zRes;
    
    //Creates an empty volume
    Volume(size_t xr, size_t yr, size_t zr);
    
    //Saves a volume
    void save(const char* filenae) const;
    
    //Array access operator
    unsigned char& operator()(size_t x, size_t y, size_t z)
    {
        assert(x < xRes && y < yRes && z < zRes);
        return data[x + xRes * (y + yRes * z)];
    }
    
    //Const version
    unsigned char operator()(size_t x, size_t y, size_t z) const
    {
        if(x < xRes && y < yRes && z < zRes)
            return data[x + xRes * (y + yRes * z)];
        return 0;
    }
    
    //Check for ray-volume test
    bool trace_ray(const Ray& r, vec3& t) const;
    
    //Check if a point is on the surface
    bool on_surface(const ivec3& x) const;
};

#endif
