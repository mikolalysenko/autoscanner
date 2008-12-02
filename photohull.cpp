#include <iostream>
#include <cmath>
#include <utility>
#include <vector>

#include "photohull.h"

using namespace std;
using namespace blitz;

//Plane directions
const vec3 
DN[6] = {
    vec3(0, 0, 1),
    vec3(0, 0,-1),
    vec3(1, 0, 0),
    vec3(-1, 0, 0),
    vec3(0, 1, 0),
    vec3(0,-1, 0),
},

DU[6] = {
    vec3(0, 1, 0),
    vec3(0, 1, 0),
    vec3(0, 1, 0),
    vec3(0, 1, 0),
    vec3(1, 0, 0),
    vec3(1, 0, 0),    
},

DV[6] = {
    vec3(1, 0, 0),
    vec3(1, 0, 0),
    vec3(0, 0, 1),
    vec3(0, 0, 1),
    vec3(0, 0, 1),
    vec3(0, 0, 1),
};

const int N_DIM[6] = { 2, 2, 0, 0, 1, 1, },
          U_DIM[6] = { 1, 1, 1, 1, 0, 0, },
          V_DIM[6] = { 0, 0, 2, 2, 2, 2, };

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
    ivec3 point,
    int d)
{
    //Create patches
    vector<Neighborhood> patches;
    
    bool in_frame = false;
    
    vec3 pt = point;
    pt += 0.5;
    
    //Construct the cone
    vec3 dn = DN[d], du = DU[d], dv = DV[d];
    
    vec4 cone[4];
    cone[0] = cone[1] = cone[2] = cone[3] = vec4(dn(0), dn(1), dn(2), 0);
    
    cone[0] += vec4(du(0), du(1), du(2), 0);
    cone[1] -= vec4(du(0), du(1), du(2), 0);
    cone[2] += vec4(dv(0), dv(1), dv(2), 0);
    cone[3] -= vec4(dv(0), dv(1), dv(2), 0);
    
    for(int i=0; i<4; i++)
    {
        cone[i](3) = -(cone[i](0) * pt(0) + 
                       cone[i](1) * pt(1) +
                       cone[i](2) * pt(2));
    }
    
    for(size_t i=0; i<views.size(); i++)
    {
        View * view = views[i];
        
        //Check for in-frame condition
        vec3 img_loc = hgmult(view->cam, pt);
        int ix = img_loc(0), iy = img_loc(1);
        if(ix < 0 || ix >= view->img->width || iy < 0 || iy >= view->img->height)
            continue;
        in_frame = true;
        
        //Check that the point is in the cone
        bool fail = false;
        for(int j=0; j<4; j++)
        {
            double d = 
                cone[j](0) * view->center(0) +
                cone[j](1) * view->center(1) +
                cone[j](2) * view->center(2) +
                cone[j][3];
            
            if(d > 0.0f)
            {
                fail = true;
                break;
            }
        }
        if(fail)
            continue;
        
        //If already consistent, then continue
        if(view->consist(ix, iy))
            continue;
        
        /*
        //Visual hull hack
        vec3 pixel = view->readPixel(ix, iy);
        if(0.3 * pixel(0) + 0.59 * pixel(1) + 0.11 * pixel(2) < 30)
            return false;
        */
        
        //Accumulate statistics
        patches.push_back(Neighborhood(view, ix, iy));
    }
    
    //If not in frame, then voxel is trivially non-consistent
    if(!in_frame || !checkNeighborhood(patches))
        return false;
    
    //Mark consistency
    for(size_t i=0; i<patches.size(); i++)
        patches[i].view->consist(patches[i].x, patches[i].y) = true;
    
    return true;
} 

//Sweeps a plane through volume
bool planeSweep(
    std::vector<View*> views,
    Volume* volume,
    int d,
    ivec3 bound)
{
    //Returns true when volume is photo consistent
    bool done = true;
    
    //Read in bounds on loops
    int si = bound[N_DIM[d]],
        sj = bound[U_DIM[d]],
        sk = bound[V_DIM[d]];
    
    //Read in delta coordinates
    vec3 dn = DN[d],
         du = DU[d],
         dv = DV[d];
    
    //Initialize p
    vec3 p = 0.0f;
    for(int i=0; i<3; i++)
        p(i) = dn(i) < 0 ? bound[i] - 1 : 0;
    
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
                {
                    if(!checkConsistency(views, volume, r, d))
                    {
                        (*volume)(r(0), r(1), r(2)) = 0;
                        done = false;
                    }
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
    
    
    while(true)
    {
        //Clear consistency data
        for(size_t i=0; i<views.size(); i++)
            views[i]->resetConsist();
        
        //Do plane sweeps
        for(int i=0; i<6; i++)
            planeSweep(views, volume, i, ivec3(xr, yr, zr));
    }
    
    return volume;
}


