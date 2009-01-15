#include <iostream>
#include <cmath>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>

#include "photohull.h"
#include "config.h"
#include "consistency.h"

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



void visualHull(
    Volume* volume, 
    std::vector<View*>& views) {

    double threshold = 5;

    int xr = volume->xRes, yr = volume->yRes, zr = volume->zRes;
    
    for (int x = 0; x < xr; x++) { 
        if (x % 5 == 0) { cout << "."; cout.flush(); }
        if (x % 10 == 0) { cout << x; cout.flush(); }
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
    vec3 p = 0.0f, q, r;
    for(int i=0; i<3; i++)
        p(i) = dn(i) < 0 ? bound[i] - 1 : 0;
    
    for(int i=0; i<si; i++, p += dn, q = p) { 
        if ((i % 10) == 0) cout << "." << flush;       
        for(int j=0; j<sj; j++, q += du, r = q) {
            for(int k=0; k<sk; k++, r += dv) {
                if(volume->on_surface(r) && !checkConsistency(views, volume, r, d)) {
                    (*volume)(r(0), r(1), r(2)) = 0;
                    removed++;
                    done = false;
                }        
            }
        }
    }

    cout << removed << endl;

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

    //visualHull(volume, views);
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
        if (pass % 5 == 2) {
            volume->save("temp/temp");
            for(size_t i=0; i<views.size(); i++) {
                char fname[1024];
                
                snprintf(fname, 1024, "temp/consist%04d.png", i);
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




