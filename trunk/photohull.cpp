#include <iostream>
#include <cmath>
#include <utility>
#include <vector>

#include "photohull.h"

using namespace std;
using namespace blitz;

//A neighborhood within a view
struct Neighborhood
{
    View * view;
    int x, y;
    
    Neighborhood() {}
    Neighborhood(const Neighborhood& n) : view(n.view), x(n.x), y(n.y) {}
    Neighborhood(View * v, int x_, int y_) : view(v), x(x_), y(y_) {}
    
};

//Check if a set of neighborhoods is photoconsistent
// This clearly does not work.
bool checkNeighborhood(vector<Neighborhood> &patches)
{
    //No patches, no match
    if(patches.size() <= 1)
        return true;
    
    vec3 mu0 = 0.0f, mu1 = 0.0f;
    
    for(size_t i=0; i<patches.size(); i++)
    {
        vec3 pixel = patches[i].view->readPixel(
            patches[i].x,
            patches[i].y);
        
        mu0 += pixel;
        pixel *= pixel;
        mu1 += pixel;
    }

    //Calculate sigma
    mu0 *= mu0;
    mu0 /= (float)patches.size();
    mu1 -= mu0;
    mu1 /= (float)(patches.size() - 1);
    
    vec3 sigma = vec3(
        sqrtf(mu1(0)),
        sqrtf(mu1(1)),
        sqrtf(mu1(2)));
    
    //These values are arbitrary
    return 
            (sigma(0) < 15) &&
            (sigma(1) < 15) &&
            (sigma(2) < 20);
}

//Checks photoconsistency of a voxel in the volume
bool checkConsistency(
    std::vector<View*> views,
    Volume* volume,
    ivec3 point)
{
    //Create patches
    vector<Neighborhood> patches;
    
    bool in_frame = false;
    
    vec3 pt = point;
    pt += 0.5;
    
    for(size_t i=0; i<views.size(); i++)
    {
        View * view = views[i];
        
        //Check for in-frame condition
        vec3 img_loc = hgmult(view->cam, pt);
        int ix = img_loc(0), iy = img_loc(1);
        if(ix < 0 || ix >= view->img->width || iy < 0 || iy >= view->img->height)
            continue;
        in_frame = true;
        
        /*
        //Visual hull hack
        vec3 pixel = view->readPixel(ix, iy);
        if(0.3 * pixel(0) + 0.59 * pixel(1) + 0.11 * pixel(2) < 30)
            return false;
        */
        
        //Do ray-volume intersection
        vec3 tmp, dir = view->center;
        dir -= pt;
        dir /= sqrtf(dir(0) * dir(0) + dir(1) * dir(1) + dir(2) * dir(2));
        if(volume->trace_ray(Ray(pt, dir), tmp))
        {
            continue;
        }
        
        //Accumulate statistics
        patches.push_back(Neighborhood(view, ix, iy));
    }
    
    //If not in frame, then voxel is trivially non-consistent
    if(!in_frame)
        return false;
    
    return checkNeighborhood(patches);
} 

//Sweeps a plane through volume
bool planeSweep(
    std::vector<View*> views,
    Volume* volume,
    ivec3 dn,
    ivec3 du,
    ivec3 dv,
    ivec3 p,
    int si, int sj, int sk)
{
    //Returns true when volume is photo consistent
    bool done = true;
    
    for(int i=0; i<si; i++)
    {
        cout << "sweeping: " << p << endl;
        
        ivec3 q = p;
        for(int j=0; j<sj; j++)
        {
            ivec3 r = q;
            for(int k=0; k<sk; k++)
            {
                if(volume->on_surface(r))
                if(!checkConsistency(views, volume, r))
                {
                    (*volume)(r(0), r(1), r(2)) = 0;
                    done = false;
                }
                
                r += dv;
            }
            q += du;
        }
        p += dn;
    
        //Save intermediate results for debugging
        volume->save("temp/temp");
    }

    return done;
}

//Finds the photo hull
Volume* findHull(
    std::vector<View*> views, 
    int xr, int yr, int zr)
{
    Volume * volume = new Volume((size_t)xr, (size_t)yr, (size_t)zr);
    
    for(int i=0; i<xr; i++)
    for(int j=0; j<yr; j++)
    for(int k=0; k<zr; k++)
        (*volume)(i,j,k) = 255;
    
    
    while(
        planeSweep(views, volume, 
            ivec3(0, 0, 1),
            ivec3(0, 1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, 0),
            zr, yr, xr) ||
    
        planeSweep(views, volume, 
            ivec3(0, 0,-1),
            ivec3(0, 1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, zr-1),
            zr, yr, xr) ||
            
        planeSweep(views, volume, 
            ivec3(1, 0, 0),
            ivec3(0, 1, 0),
            ivec3(0, 0, 1),
            ivec3(0, 0, 0),
            xr, yr, zr) ||

        planeSweep(views, volume, 
            ivec3(-1, 0, 0),
            ivec3(0, 1, 0),
            ivec3(0, 0, 1),
            ivec3(xr-1, 0, 0),
            xr, yr, zr) ||
            
        planeSweep(views, volume, 
            ivec3(0, 1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, 1),
            ivec3(0, 0, 0),
            yr, xr, zr) ||

        planeSweep(views, volume, 
            ivec3(0,-1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, 1),
            ivec3(0,yr-1, 0),
            yr, xr, zr))
    {
        cout << "redo sweep" << endl;
    }
    
    return volume;
}


