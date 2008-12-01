/**
 * Volume data implementation
 */

//STL
#include <cstdlib>
#include <cstring>
#include <sstream>

//Project
#include "misc.h"
#include "volume.h"

using namespace std;
using namespace blitz;

//Volume constructor
Volume::Volume(size_t x, size_t y, size_t z) :
    xRes(x), yRes(y), zRes(z)
{
    data = new unsigned char[xRes * yRes * zRes];
    memset(data, 0, xRes * yRes * zRes);
}

//Saves volume data to file
void Volume::save(const char * file) const
{
    //Allocate image header
    IplImage * img = cvCreateImageHeader(cvSize(xRes, yRes), IPL_DEPTH_8U, 1);
    img->widthStep = xRes;
    
    for(size_t i=0; i<zRes; i++)
    {
        //Set data pointer
        img->imageData = (char*)&data[i * xRes * yRes];
        
        //Compute file name
        char buf[1024];
        snprintf(buf, 1024, "%s-%04d.png", file, i);
        
        //Save image
        cvSaveImage(buf, img); 
    }
    
    //Release image header
    cvReleaseImageHeader(&img);
}


//Trace a ray through the volume
bool Volume::trace_ray(const Ray& r, vec3& x) const
{
    //Extract int and frac parts
    x = r.o;
    
    while(true)
    {
        float t = 1e80;
        
        for(int i=0; i<3; i++)
        {
            float f = 1.0;
            if(r.d[i] > 1e-10)
                f = (1.0f + floor(x[i]) - x[i]) / r.d[i];
            else if(r.d[i] < -1e-10)
                f = (x[i] - floor(x[i])) / r.d[i];
            if(f < 1e-10)
                f = 1.0;
            t = min(t, f);
        }

        for(int i=0; i<3; i++)
            x[i] = x[i] + t * r.d[i];
        
        //Check bounds
        if(
            x[0] < 0 || x[0] >= xRes ||
            x[1] < 0 || x[1] >= yRes ||
            x[2] < 0 || x[2] >= zRes)
            break;
            
        //Check hit
        if((*this)(x[0], x[1], x[2]))
            return true;
    }


    return false;
}

//Checks if a point is on the surface
bool Volume::on_surface(const ivec3& x) const
{
    int ix = x(0),
        iy = x(1),
        iz = x(2);
    
    return (*this)(ix, iy, iz) && !(
        (*this)(ix+1, iy, iz) &&
        (*this)(ix-1, iy, iz) &&
        (*this)(ix, iy+1, iz) &&
        (*this)(ix, iy-1, iz) &&
        (*this)(ix, iy, iz+1) &&
        (*this)(ix, iy, iz-1));
}
