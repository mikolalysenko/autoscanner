#include <iostream>
#include <cmath>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>

#include "photohull.h"

using namespace std;
using namespace blitz;

long long count1 = 0, count2 = 0, count3 = 0, count4 = 0, count5 = 0, count6 = 0;

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
        int pad = 0;
        x_min = * min_element(verts[0], verts[0] + 8) - pad;
        y_min = * min_element(verts[1], verts[1] + 8) - pad;
        x_max = * max_element(verts[0], verts[0] + 8) + pad;
        y_max = * max_element(verts[1], verts[1] + 8) + pad;
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

void visualHull(
    Volume* volume, 
    std::vector<View*> views) {

    double threshold = 5;

    int xr = volume->xRes, yr = volume->yRes, zr = volume->zRes;
    
    for (int x = 0; x < xr; x++) { 
        if (x % 10 == 0) { cout << "."; cout.flush(); }
        if (x % 50 == 0) { cout << x; cout.flush(); }
        for (int y = 0; y < yr; y++) {
            for (int z = 0; z < zr; z++) {
                vec3 pt(x + 0.5, y + 0.5, z + 0.5);
                int num_views = 0, mark = 0;
                for (int i = 0; i < views.size(); i++) {
                    vec3 proj = hgmult(views[i]->cam, pt);
                    if (!views[i]->in_bounds(proj(0), proj(1))) continue;
                    num_views++;
                    
                    vec3 pixel = views[i]->readPixel(proj(0), proj(1));
                    if ((pixel(0) + pixel(1) + pixel(2)) / 3 < threshold) mark++;
                }
                
                if (mark > 2 || mark * 2 >= num_views) (*volume)(x,y,z) = 0;

            }
        }
    }

    cout << endl;


}

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

    //No patches, no match
    if(patches.size() <= 1)
        return true;

    //Calculate sigma
    mu0 *= mu0;
    mu0 /= (float)patches.size();
    mu1 -= mu0;
    mu1 /= (float)(patches.size() - 1);
    
    vec3 sigma = mu1;
    //These values are arbitrary
    vec3 thresh(5000, 5000, 5000 );
    bool cull1 = sigma(0) > thresh(0), 
        cull2 = sigma(1) > thresh(1),
        cull3 = sigma(2) > thresh(2);
    count1 += cull1 && !cull2 && !cull3;
    count2 += !cull1 && cull2 && !cull3;
    count3 += !cull1 && !cull2 && cull3;
    count4 += cull1 || cull2 || cull3;
    return  !cull1 && !cull2 && !cull3;
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
    
    vec3 pt = point; pt += 0.5;
    
    //Construct the cone
    vec3 dn = DN[d], du = DU[d], dv = DV[d];
    
    Cone cone(dn, du, dv, pt);
    int mark = 0;
    
    //Traverse all views
    for(size_t i=0; i<views.size(); i++)
    {
        View * view = views[i];
        
        //Check for in-frame condition
        vec3 img_loc = hgmult(view->cam, pt);
        int ix = img_loc(0), iy = img_loc(1);
        
        if (!view->in_bounds(ix, iy) || cone.contains(view->center)) continue;
        
        in_frame = true;
        
        //If already consistent, then continue
        if(view->consist(ix, iy))
            continue;
        
        //Accumulate statistics
        patches.push_back(Neighborhood(view, ix, iy));
    }
    
    if(!in_frame || !checkNeighborhood(patches))
        return false;
    
    //Mark consistency
    for(size_t i=0; i<patches.size(); i++) {
        VoxelProjection vp(patches[i].view, point);
        for (pair<int, int> iter = vp.begin(); iter != vp.end(); vp.next(iter))
            patches[i].view->consist(iter.first, iter.second) = 255;
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
    for(size_t t=0; t<views.size(); t++) {
        views[t]->resetConsist();
    }
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

    visualHull(volume, views);
    while(true)
    {
        num_removed = 0;
        cout << "Pass #" << pass++ << endl;        
        
        //Do plane sweeps
        for(int i=0; i<6; i++) {
            cout << i << " " << flush;
            planeSweep(views, volume, i, ivec3(xr, yr, zr), num_removed);
        }
        cout << endl;
        
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
        
        cout << count1 << " " << count2 << " " << count3 << " " << count4 << " "
             << count5 << " " << count6 << endl;
        if (num_removed == 0) break;
    }
    
    return volume;
}




