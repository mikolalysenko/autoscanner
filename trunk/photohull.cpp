#include <iostream>
#include <cmath>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>

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

struct Cone
{
    vec4 cone[4];

    Cone(vec3 dn, vec3 du, vec3 dv, vec3 pt) {
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
    }

    bool contains(vec3 pt) {
        for(int j=0; j<4; j++)
        {
            double d = 
                cone[j](0) * pt(0) +
                cone[j](1) * pt(1) +
                cone[j](2) * pt(2) +
                cone[j][3];
            
            if(d > 0.0f)
                return false;
        }
    }

};

struct VoxelProjection
{
    View* view;
    double verts[2][8];
    int x_min, x_max, y_min, y_max;

    static void clamp(int& v, int min, int max) {
        v = v < min ? min : v > max ? max : v;
    }

    VoxelProjection(View* v, vec3 pt) : view(v) {
        vec3 offset[8] = {
            vec3(0, 0, 0),
            vec3(1, 0, 0),
            vec3(0, 1, 0),
            vec3(0, 0, 1),
            vec3(0, 1, 1),
            vec3(1, 0, 1),
            vec3(1, 1, 0),
            vec3(1, 1, 1)
            };
        for (int i = 0; i < 8; i++) {
            vec3 v = pt; v += offset[i];
            vec3 proj = hgmult(view->cam, v);
            verts[0][i] = proj(0); verts[1][i] = proj(1);
        }
        x_min = * min_element(verts[0], verts[0] + 8);
        y_min = * min_element(verts[1], verts[1] + 8);
        x_max = * max_element(verts[0], verts[0] + 8);
        y_max = * max_element(verts[1], verts[1] + 8);
        assert(x_min <= x_max);
        assert(y_min <= y_max);
        clamp(x_min, 0, view->img->width - 1); clamp(x_max, 0, view->img->width - 1);
        clamp(y_min, 0, view->img->height - 1); clamp(y_max, 0, view->img->height - 1);
    }
	
    pair<int, int> begin() const {
        return pair<int, int>(x_min, y_min);
    }
    pair<int, int> end() const {
        return pair<int, int>(x_min, y_max + 1);
    }
    void next(pair<int, int>& p) const {
        if (p.first == x_max) 
            { p.first = x_min; p.second++; }
        else
            p.first++;
    }
};

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
    if(patches.size() < 1)
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
    
    vec3 mu2 = mu0; 
    mu2 /= (float)patches.size();

    //if (patches.size() == 1 && mu2(0) + mu2(1) + mu2(2) < 40) return false;

    //No patches, no match
    if(patches.size() <= 1)
        return true;

    //Calculate sigma
    mu0 *= mu0;
    mu0 /= (float)patches.size();
    mu1 -= mu0;
    mu1 /= (float)(patches.size() - 1);
    
   /* vec3 sigma = vec3(
        sqrtf(mu1(0)),
        sqrtf(mu1(1)),
        sqrtf(mu1(2))); */
    vec3 sigma = mu1;
    //These values are arbitrary
    return  (sigma(0) < 4000) &&
            (sigma(1) < 8000) &&
            (sigma(2) < 16000);
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
    
    Cone cone(dn, du, dv, pt);
    
    //Traverse all views
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

        //if(!cone.contains(view->center))
        //    continue;
        
        //If already consistent, then continue
        if(view->consist(ix, iy))
            continue;
        

        //Visual hull hack
        vec3 pixel = view->readPixel(ix, iy);
        if(0.3 * pixel(0) + 0.59 * pixel(1) + 0.11 * pixel(2) < 2)
            return false;
        
        //Accumulate statistics
        patches.push_back(Neighborhood(view, ix, iy));
    }
    
    //If not in frame, then voxel is trivially non-consistent
    if(!in_frame || !checkNeighborhood(patches))
        return false;
    
    //Mark consistency
    for(size_t i=0; i<patches.size(); i++) {
        VoxelProjection vp(patches[i].view, point);
        for (pair<int, int> iter = vp.begin(); iter != vp.end(); vp.next(iter))
            patches[i].view->consist(iter.first, iter.second) = 255;
    }
    
    //Mark checked
    if(patches.size() > 0)
    {
        (*volume)(point(0), point(1), point(2)) &= ~(1 << d);
    }
    
    return true;
} 

//Sweeps a plane through volume
bool planeSweep(
    std::vector<View*> views,
    Volume* volume,
    int d,
    ivec3 bound,
    int& removed)
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
    
    cout << "sweeping " << dn << endl;
    for(int i=0; i<si; i++)
    {        
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
                        removed++;
                        done = false;
                    }
                }
                r += dv;
            }
            q += du;
        }
        p += dn;
    }

    return done;
}

//Finds the photo hull
Volume* findHull(
    std::vector<View*> views, 
    int xr, int yr, int zr,
    vec3 low, vec3 high)
{
    Volume * volume = new Volume((size_t)xr, (size_t)yr, (size_t)zr, low, high);
    
    for(int i=0; i<xr; i++)
    for(int j=0; j<yr; j++)
    for(int k=0; k<zr; k++)
        (*volume)(i,j,k) = 255;
    
    int pass = 1; int num_removed;
    while(true)
    {
        num_removed = 0;
        cout << "Pass #" << pass++ << endl;
        //Clear consistency data
        for(size_t i=0; i<views.size(); i++) {
            views[i]->resetConsist();
        }
        
        //Do plane sweeps
        for(int i=0; i<6; i++)
            planeSweep(views, volume, i, ivec3(xr, yr, zr), num_removed);
        
        //Mark the pixels from multiview volumes
        //Unimplemented

        //Save intermediate results for debugging
        if (pass % 3 == 2) {
            volume->save("temp/temp");
            for(size_t i=0; i<views.size(); i++) {
                string fname = "temp/consist"; fname += '0' + i; fname += ".png";
                views[i]->writeConsist(fname);
            }
        }

        cout << "end pass. removed " << num_removed << " voxels" << endl;
        if (num_removed == 0) break;
        
        //Reset volumes 
        
        /*
        for(int i=0; i<xr; i++)
        for(int j=0; j<yr; j++)
        for(int k=0; k<zr; k++)
            if((*volume)(i,j,k))
                (*volume)(i,j,k) = 255;
        */ 
    }
    
    return volume;
}


