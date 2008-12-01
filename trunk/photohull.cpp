#include <iostream>
#include <cmath>
#include <utility>
#include <vector>

#include "photohull.h"

using namespace std;
using namespace blitz;

//Number of ransac tries
#define RANSAC_TRIES    1024
#define N_X             5
#define N_Y             5
#define THRESHOLD       55


//vec3 CHI;


//A neighborhood within a view
struct Neighborhood
{
    View * view;
    int x, y;
    
    Neighborhood() {}
    Neighborhood(const Neighborhood& n) : view(n.view), x(n.x), y(n.y) {}
    Neighborhood(View * v, int x_, int y_) : view(v), x(x_), y(y_) {}
    
};

/*
float color_dist(const vec3& a, const vec3& b)
{
    vec3 d = a;
    d -= b;
    return d(0) * d(0) + d(1) * d(1) + d(2) * d(2);
}
*/

/*
void findChi(vector<View*> views)
{
	float k = 0.0f;
	vec3 mu0 = 0.0f, mu1 = 0.0f;
	
	for(size_t i=0; i<views.size(); i++)
	for(int x=0; x<views[i]->img->width; x++)
	for(int y=0; y<views[i]->img->height; y++)
	{
		vec3 pixel = views[i]->readPixel(x, y);
		k++;
		mu0 += pixel;
		pixel *= pixel;
		mu1 += pixel;
	}

    mu0 *= mu0;
    mu0 /= k;
    mu1 -= mu0;
	
	CHI = vec3(
		sqrtf(mu1(0)),
		sqrtf(mu1(1)),
		sqrtf(mu1(2)));
		
	CHI = mu1;
}
*/



//Check if a set of neighborhoods is photoconsistent
bool checkNeighborhood(vector<Neighborhood> &patches)
{
    //No patches, no match
    if(patches.size() <= 1)
        return true;
    
    //cout << "Checking patch..." << endl;
    
	/*
    //Try doing RANSAC a fixed number of times    
    for(int r=0; r<=256; r+=50)
    for(int g=0; g<=256; g+=50)
    for(int b=0; b<=256; b+=50)
    {
        //1. Select random pixel in neighborhood of random patch
        vec3 sample_pixel = vec3(r, g, b);
        
        //cout << "base_pixel = " << sample_pixel << endl;
        
        //2. Find set of pixels closest to given pixel in all patches
        vec3 mu0 = 0.0f, mu1 = 0.0f;
        for(size_t j=0; j<patches.size(); j++)
        {
            vec3 closest_pixel = 0.0f;
            float dist = 1e30;
            
            for(int dx=-N_X; dx<=N_X; dx++)
            for(int dy=-N_Y; dy<=N_Y; dy++)
            {
                vec3 pixel = patches[j].view->readPixel(
                    patches[j].x + dx,
                    patches[j].y + dy);
                
                float d = color_dist(pixel, closest_pixel);
                
                if(d < dist)
                {
                    dist = d;
                    closest_pixel = pixel;
                }
            }
            
            
            //Adjust moments
            mu0 += closest_pixel;
            closest_pixel *= closest_pixel;
            mu1 += closest_pixel;
        }
        
        //cout << "mu0 = " << mu0 << endl;
        //cout << "mu1 = " << mu1 << endl;
        
        //Calculate sigma
        mu0 *= mu0;
        mu0 /= (float)patches.size();
        mu1 -= mu0;
        mu1 /= (float)(patches.size() - 1);
        
        vec3 sigma = vec3(
            sqrtf(mu1(0)),
            sqrtf(mu1(1)),
            sqrtf(mu1(2)));
        
        if(sigma(0) < THRESHOLD &&
            sigma(1) < THRESHOLD &&
            sigma(2) < THRESHOLD)
            return true;
    }
    
    return false;
	*/

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
    
	/*
    return 
		sigma(0) < CHI(0) &&
		sigma(1) < CHI(1) &&
		sigma(2) < CHI(2);
	return 
		(mu1(0) / CHI(0) < 1) &&
		(mu1(1) / CHI(1) < 1) &&
		(mu1(2) / CHI(2) < 1);
	*/
	
	//cout << "mu = " << mu1 << endl;
	//cout << "sigma = " << sigma << endl;
	
	return 
		(sigma(0) < 20) &&
		(sigma(1) < 25) &&
		(sigma(2) < 40);
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
        
        //Try reading a color from the image
        vec3 img_loc = hgmult(view->cam, pt);
        
        int ix = img_loc(0), iy = img_loc(1);
        
        if(ix < 0 || ix >= view->img->width || iy < 0 || iy >= view->img->height)
            continue;
			
		/*
		vec3 pixel = view->readPixel(ix, iy);
		if(0.3 * pixel(0) + 0.59 * pixel(1) + 0.11 * pixel(2) < 30)
			return false;
		*/
		
        in_frame = true;
        
        //Do ray-volume intersection
        vec3 tmp, dir = view->center;
        dir -= pt;
        dir /= sqrtf(dir(0) * dir(0) + dir(1) * dir(1) + dir(2) * dir(2));
        if(volume->trace_ray(Ray(pt, dir), tmp))
        {
            /*
            cout << "blocked:  view = " << i << endl;
            cout << "point= " << pt << endl;
            cout << "centr= " << view->center << endl;
            cout << "dir  = " << dir << endl;
            cout << "hit  = " << tmp << endl;
            */
            continue;
        }
        
        //Accumulate statistics
        patches.push_back(Neighborhood(view, ix, iy));
    }
    
    if(!in_frame)
        return false;
    
    return checkNeighborhood(patches);
} 




//Checks photoconsistency of a voxel in the volume
bool checkConsistencyCone(
    std::vector<View*> views,
    Volume* volume,
    ivec3 point,
    ivec3 dn,
    ivec3 du,
    ivec3 dv)
{
    
    //Create patches
    vector<Neighborhood> patches;
    
    vec3 pt = point;
    pt += 0.5;
    
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
    
    bool in_frame = false;
    
    for(size_t i=0; i<views.size(); i++)
    {
        View * view = views[i];
        
        //Try reading a color from the image
        vec3 img_loc = hgmult(view->cam, pt);
        
        int ix = img_loc(0), iy = img_loc(1);
        
        if(ix < 0 || ix >= view->img->width || iy < 0 || iy >= view->img->height)
            continue;
        
		vec3 pixel = view->readPixel(ix, iy);
		if(pixel(0) + pixel(1) + pixel(2) < 30)
			return false;
		
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
            
            if(d > 0.001)
            {
                fail = true;
                break;
            }
        }
        if(fail)
			continue;
        
        
        //Do ray-volume intersection
        vec3 tmp, dir = view->center;
        dir -= pt;
        dir /= sqrtf(dir(0) * dir(0) + dir(1) * dir(1) + dir(2) * dir(2));
        if(volume->trace_ray(Ray(pt, dir), tmp))
        {
            /*
            cout << "blocked:  view = " << i << endl;
            cout << "point= " << pt << endl;
            cout << "centr= " << view->center << endl;
            cout << "dir  = " << dir << endl;
            cout << "hit  = " << tmp << endl;
            */
            continue;
        }
        
        //Accumulate statistics
        patches.push_back(Neighborhood(view, ix, iy));
    }
    
    if(!in_frame)
        return false;
    
	//cout << "number of views = " << patches.size() << endl;
	
    return checkNeighborhood(patches);
    
    /*

    if(k <= 1)
        return true;
    
    //TODO:  Is this correlation function correct?
    float val = 
        0.3  * sqrt((col_2(0) - col(0) * col(0) / (float)k) / (float)(k - 1)) +
        0.59 * sqrt((col_2(1) - col(1) * col(1) / (float)k) / (float)(k - 1)) +
        0.11 * sqrt((col_2(2) - col(2) * col(2) / (float)k) / (float)(k - 1));
    */
} 



void planeSweep(
    std::vector<View*> views,
    Volume* volume,
    ivec3 dn,
    ivec3 du,
    ivec3 dv,
    ivec3 p,
    int si, int sj, int sk)
{
    for(int i=0; i<si; i++)
    {
        cout << "sweeping: " << p << endl;
        
        ivec3 q = p;
        for(int j=0; j<sj; j++)
        {
            ivec3 r = q;
            for(int k=0; k<sk; k++)
            {
				//Check 
				if(volume->on_surface(r))
				{
					if(!checkConsistency(views, volume, r))
					{
						(*volume)(r(0), r(1), r(2)) = 0;
					}
				}
                r += dv;
            }
            q += du;
        }
        p += dn;
    
    volume->save("temp/temp");
    }

}

Volume* findHull(
    std::vector<View*> views, 
    int xr, int yr, int zr)
{
	//findChi(views);

    Volume * volume = new Volume((size_t)xr, (size_t)yr, (size_t)zr);
    
    for(int i=0; i<xr; i++)
    for(int j=0; j<yr; j++)
    for(int k=0; k<zr; k++)
        (*volume)(i,j,k) = 255;
    
    
    while(true)
    {
        planeSweep(views, volume, 
            ivec3(0, 0, 1),
            ivec3(0, 1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, 0),
            zr, yr, xr);
            
        planeSweep(views, volume, 
            ivec3(0, 0,-1),
            ivec3(0, 1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, zr-1),
            zr, yr, xr);
            
            
        planeSweep(views, volume, 
            ivec3(1, 0, 0),
            ivec3(0, 1, 0),
            ivec3(0, 0, 1),
            ivec3(0, 0, 0),
            xr, yr, zr);

        planeSweep(views, volume, 
            ivec3(-1, 0, 0),
            ivec3(0, 1, 0),
            ivec3(0, 0, 1),
            ivec3(xr-1, 0, 0),
            xr, yr, zr);
            
        planeSweep(views, volume, 
            ivec3(0, 1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, 1),
            ivec3(0, 0, 0),
            yr, xr, zr);

        planeSweep(views, volume, 
            ivec3(0,-1, 0),
            ivec3(1, 0, 0),
            ivec3(0, 0, 1),
            ivec3(0,yr-1, 0),
            yr, xr, zr);

        //Check convergence
        bool resolved = true;

        for(int i=0; i<xr; i++)
        for(int j=0; j<yr; j++)
        for(int k=0; k<zr; k++)
        {
			if((*volume)(i,j,k) == 254)
				(*volume)(i,j,k)++;
            
			if((*volume)(i,j,k) > 0 && (*volume)(i,j,k) < 254)
            {
			    //Check consistency
                if(checkConsistency(views, volume, vec3(i,j,k)))
				{
					(*volume)(i,j,k) = 255;
				}
				else
                {
                    resolved = false;
                }
            }
        }
        
        if(resolved)
            return volume;

        for(int i=0; i<xr; i++)
        for(int j=0; j<yr; j++)
        for(int k=0; k<zr; k++)
            if((*volume)(i,j,k) < 255)
				(*volume)(i,j,k) = 0;
    }
}


