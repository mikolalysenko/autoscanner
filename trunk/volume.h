#ifndef VOLUME_H
#define VOLUME_H

#include <cassert>
#include <string>

#include "misc.h"

using namespace blitz;


//Voxel data structure
struct Volume
{
    unsigned char * data;
    size_t xRes, yRes, zRes;

    vec3 low, high;
    
    //Creates an empty volume
    Volume(size_t xr, size_t yr, size_t zr, vec3 l = vec3(0,0,0), vec3 h = vec3(1,1,1));
    
    //Saves a volume
    void save(const char* filenae) const;
    void savePly(const std::string& filename) const;
    
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
    bool on_surface(int x, int y, int z) const;

    //Get 3d position of a voxel
    vec3 pos_3d(int x, int y, int z) const;
};

#endif
