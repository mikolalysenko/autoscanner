/**
 * Volume data implementation
 */

//STL
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>

//Project
#include "volume.h"

//TODO: Switch all instances of variables to Eigen style
#include <Eigen/Core>
USING_PART_OF_NAME_NAMESPACE_EIGEN

typedef Vector2f  vec2;
typedef Vector2i ivec2;
typedef Vector3f  vec3;
typedef Vector3i ivec3;
typedef Vector4f  vec4;
typedef Vector4i ivec4;
typedef Matrix4f mat44;


using namespace std;

//Volume constructor
Volume::Volume(size_t x, size_t y, size_t z, vec3 l, vec3 h) :
    xRes(x), yRes(y), zRes(z), low(l), high(h)
{
    data = new unsigned char[xRes * yRes * zRes];
    color = new unsigned char[3 * xRes * yRes * zRes];

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

   // savePly(file);
}

void Volume::savePly(const std::string& file, const vector<View*> views) const {
    std::string filename = file + "-o.ply";

    ifstream fin((file + "-o.ply").c_str());
    ofstream fbout((file + "-o2.ply").c_str(), ios_base::out | ios_base::trunc);
    fbout << fin.rdbuf() << endl; fin.close(); fbout.close();

    std::vector<vec3> points;
    std::vector<ivec3> colors;

    for (size_t x = 0; x < xRes; x++)
    for (size_t y = 0; y < yRes; y++)
    for (size_t z = 0; z < zRes; z++) {
        if (near_surface(x, y, z) != 0) {
            vec3 pos = pos_3d(
                (double)x + drand48(),
                (double)y + drand48(),
                (double)z + drand48());

            unsigned char* pix = pixel(x, y, z);

            cout << pix[0] << " " << pix[1] << " " << pix[2] << endl;
            
            ivec3 color; for (int c = 0; c < 3; c++) color[c] = pix[c];
            
            points.push_back(pos);
            colors.push_back(color);
        }
    }
    
    ::savePly(filename, points, colors);
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
    
    return on_surface(ix, iy, iz);
}

bool Volume::on_surface(int ix, int iy, int iz) const {    
    return (*this)(ix, iy, iz) && !(
        (*this)(ix+1, iy, iz) &&
        (*this)(ix-1, iy, iz) &&
        (*this)(ix, iy+1, iz) &&
        (*this)(ix, iy-1, iz) &&
        (*this)(ix, iy, iz+1) &&
        (*this)(ix, iy, iz-1));
}

bool Volume::exterior(int ix, int iy, int iz) const {
    return !(*this)(ix, iy, iz);
}

bool Volume::near_surface(int ix, int iy, int iz) const {
    if (exterior(ix,iy,iz)) return false;
    int offset[8][3] = {
        {3,0,0},
        {-3,0,0},
        {0,3,0},
        {0,-3,0},
        {0,0,3},
        {0,0,-3}};
    for (int i = 0; i < 6; i++) {
        if (exterior(ix + offset[i][0], iy + offset[i][1], iz + offset[i][2])) return true;
    }
    return false;

}

vec3 Volume::pos_3d(int x, int y, int z) const {
    return vec3(((float)x / xRes * (high(0) - low(0))),
                ((float)y / yRes * (high(1) - low(1))),
                ((float)z / zRes * (high(2) - low(2))));
}