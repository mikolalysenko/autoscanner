#include "misc.h"

#include "view.h"
#include "volume.h"
#include "consistency.h"
#include "config.h"
#include "photohull.h"

#include <cmath>

using namespace std;
using namespace cfg;

//Check if a set of neighborhoods is photoconsistent
bool checkNeighborhood(vector<VoxelProjection> &patches, vec3& color)
{
    //No patches, no match
    if(patches.size() < 1)
        return true;
    vec3 mu0 = 0.0f, mu1 = 0.0f;
    
    for(size_t i=0; i<patches.size(); i++)
    {
        vec3 pixel = patches[i].view->readPixel(
            patches[i].x_center,
            patches[i].y_center);
        mu0 += pixel;
        pixel *= pixel;
        mu1 += pixel;
    }

    //No patches, no match
    if(patches.size() <= 1)
        return true;

    color = mu0;
    color /= (float)patches.size();
    //Calculate sigma
   // mu0 *= mu0;
    //mu0 /= (float)patches.size();

    mu0 = mu0 * mu0 / (float)patches.size();
    
    mu1 -= mu0;
    mu1 /= (float)(patches.size() - 1);
    
    vec3 sigma = mu1;
    //These values are arbitrary
    vec3 thresh = cfg::config::global.get<vec3>("photohull_threshold");

    bool cull1 = sigma(0) > thresh(0), 
        cull2 = sigma(1) > thresh(1),
        cull3 = sigma(2) > thresh(2);
    //count1 += cull1 && !cull2 && !cull3;
    //count2 += !cull1 && cull2 && !cull3;
    //count3 += !cull1 && !cull2 && cull3;
    //count4 += cull1 || cull2 || cull3;
    return  !cull1 && !cull2 && !cull3;
}

bool checkApproximateConsistency(vector<VoxelProjection>& projections)
{
    if (projections.size() < 2) return true;
    
    for (int i = 0; i < projections.size(); i++) {
        projections[i].set_pad(config::global.get<int>("approx_pad"));
    }
    
    vector<vector<float> > colors[3];

    for (int c = 0; c < 3; c++) {
        colors[c].resize(projections.size());
    }

    // I believe opencv holds images in bgr format
    for (int i = 0; i < projections.size(); i++) {
        for (pair<int, int> iter = projections[i].begin(); iter != projections[i].end(); projections[i].next(iter)) {
            vec3 pixel = projections[i].view->readPixel(iter.first, iter.second);

            for (int c = 0; c < 3; c++) {
                colors[c][i].push_back(pixel(c));
            }
        }
    }

    float sentinel = 1e20;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < colors[i].size(); j++) {
            sort(colors[i][j].begin(), colors[i][j].end());
            if (j != 0) colors[i][j].push_back(sentinel);
        }
    }

    float min_variance[3];
    
    for (int c = 0; c < 3; c++) {
        min_variance[c] = 1e30;
    
        vector<int> indices(colors[c].size(), 0);
        
        for (int t = 0; t < colors[c][0].size() - 1; t++) {
            indices[0] = t;
            for (int i = 1; i < colors[c].size(); i++) {
                for (float comp = colors[c][0][t]; 
                    fabs(comp - colors[c][i][indices[i]]) > fabs(comp - colors[c][i][indices[i] + 1]);
                    indices[i]++) 
                { }
            }

            float variance, sum, sum_squared_diff, mean;


            sum = 0;
            for (int i = 0; i < colors[c].size(); i++) {
                sum += colors[c][i][indices[i]];
                //cout << c << " --- " << i << " " << indices[i] << endl;
            }

            mean = sum / colors[c].size();
            
            sum_squared_diff = 0;
            for (int i = 0; i < colors[c].size(); i++) {
                float diff = colors[c][i][indices[i]] - mean;
                sum_squared_diff += diff * diff;
            }

            variance = sum_squared_diff / (colors[c].size() - 1);

            min_variance[c] = min(min_variance[c], variance);
            //cout << sum << " " << mean << " " <<  variance << " ~~ " << endl;

            if (min_variance[c] == 0) break;
        }

    }
    
    vec3 threshold = config::global.get<vec3>("approx_consistency_threshold");

    //for (int i = 0; i < 3; i++)
    //    cout << min_variance[i] << " ";
    //cout << endl;

    
    return min_variance[0] < threshold[0]
        && min_variance[1] < threshold[1]
        && min_variance[2] < threshold[2];        
}

    

//Checks photoconsistency of a voxel in the volume
bool checkConsistency(
    std::vector<View*>& views,
    Volume* volume,
    ivec3 point,
    int d)
{
    //Create patches
    vector<VoxelProjection> patches;
    
    bool in_frame = false;
    
    vec3 pt = point; pt += 0.5;
    
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
        
        if (!view->in_bounds(ix, iy) || cone.contains(view->center)) continue;
        
        in_frame = true;
        
        //If already consistent, then continue
        if(view->consist(ix, iy))
            continue;
        
        //Accumulate statistics
        patches.push_back(VoxelProjection(view, point));
    }
    

    vec3 img_pix;
    if(!in_frame || !checkNeighborhood(patches, img_pix))
        return false;

    
    
    //Mark consistency
    for(size_t i=0; i<patches.size(); i++) {
        
        VoxelProjection& vp = patches[i];
        unsigned char* pix = volume->pixel(point(0), point(1), point(2));
        if (i == 0) { 
            //cout << img_pix;
            pix[0] = img_pix(0);
            pix[1] = img_pix(1);
            pix[2] = img_pix(2);
        }
        for (pair<int, int> iter = vp.begin(); iter != vp.end(); vp.next(iter))
            patches[i].view->consist(iter.first, iter.second) = 255;
    }
    
    return true;
} 
