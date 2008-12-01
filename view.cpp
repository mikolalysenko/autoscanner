#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>

#include "misc.h"
#include "view.h"

using namespace std;
using namespace blitz;


void threshold(IplImage * img)
{
    //Threshold image
    for(int i=0; i<img->width * img->height; i+=3)
    {
        float intensity = 
            0.3f  * (float)img->imageData[i] + 
            0.59f * (float)img->imageData[i+1] +
            0.11f * (float)img->imageData[i+2];
        
        intensity /= 255.0f;
        
        if(intensity <= 0.19f)
            img->imageData[i] = 
            img->imageData[i+1] =
            img->imageData[i+2] = 0;
    }
}

//View constructor
View::View(IplImage * pic, mat44 K, mat44 R, mat44 S)
{
    assert(img != NULL);
    
    //Apply filters
	threshold(pic);
    img = cvCreateImage(cvSize(pic->width, pic->height), IPL_DEPTH_8U, 3);
    cvSmooth(pic, img, CV_GAUSSIAN, 3, 3, 1.5);
    
    //Calculate camera matrices
    cam = mmult(K, mmult(R, S));
    cam_inv = inverse(cam);
    
    //Compute optical center
    center = hgmult(inverse(mmult(R, S)), vec3(0, 0, 0));

    cout << "Camera: " << cam << endl
         << "K = " << K << endl
         << "R = " << R << endl
         << "S = " << S << endl
         << "CamInv: " << cam_inv << endl
         << "Center = " << center << endl;
    
    cout << "BoxCenter = " << 
        hgmult(inverse(S), vec3(0,0,0)) << endl;
    
    

}

//Read a pixel from the view
vec3 View::readPixel(int ix, int iy) const
{
    //Clamp coordinates
    ix = min(max(ix, 0), img->width - 1);
    iy = min(max(iy, 0), img->height - 1);
    
    //Read from image
    unsigned char * ptr = reinterpret_cast<unsigned char*>(&img->imageData[3 * (ix + iy * img->width)]);
    
    //Convert pixel coordinates
    return vec3(ptr[0], ptr[1], ptr[2]);
}

//Pixel reading code
bool View::readFiltered(const vec3& point, vec3& color) const
{
    //Read in homogeneous coordinates
    vec3 img_loc = hgmult(cam, point);
    
    //Get integer part of pixel coordinates
    int ix = img_loc(0), iy = img_loc(1);
    
    /*
    cout << "pix: " << ix << "," << iy << endl;
    cout << "dim: " << img->width << "," << img->height << endl;
    */
    
    //Do bounds check
    if(ix < 0 || ix >= img->width || iy < 0 || iy >= img->height)
        return false;

    //Compute average color
    color = vec3(0, 0, 0);
    
    for(float dx=-0.5; dx<=0.5; dx+=0.25)
    for(float dy=-0.5; dy<=0.5; dy+=0.25)
    for(float dz=-0.5; dz<=0.5; dz+=0.25)
    {
        vec3 npoint = vec3(
            point(0) + dx,
            point(1) + dy,
            point(2) + dz);
        
        img_loc = hgmult(cam, npoint);
        
        vec3 pix = readPixel(img_loc(0), img_loc(1));
        
        pix /= 125.0f;
        
        color += pix;
    }
    
    return true;
}


//Loads up a set of views
vector<View*> loadViews(const char * filename, vec3 lo, vec3 hi, ivec3 box)
{
    ifstream fin(filename);
    
    //Build box matrix
    mat44 S;
    for(int i=0; i<4; i++)
    for(int j=0; j<4; j++)
        S(i,j) = 0;
    for(int i=0; i<3; i++)
        S(i,i) = (hi(i) - lo(i)) / (float)(box(i) - 1);
    S(3,3) = 1.0f;
    for(int i=0; i<3; i++)
        S(i,3) = lo(i);
    
    //Read in number of files
    size_t n_files;
    fin >> n_files;
    
    //Read in view data
    vector<View*> result(n_files);
    for(size_t k=0; k<n_files; k++)
    {
        cout << "Reading camera: " << k << endl;
        
        string file;
        fin >> file;
        
        //Read K,R,T from file
        mat44 K, R;
        for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            K(i,j) = R(i,j) = 0.0f;
        for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
            fin >> K(i,j);
        
        for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
            fin >> R(i,j);
        for(int i=0; i<3; i++)
        {
            fin >> R(i,3);
        }
        R(3,3) = K(3,3) = 1.0f;
        
        //Fix up matrix
        K(0,3) = K(0,2);
        K(1,3) = K(1,2);
        K(0,2) = K(1,2) = 0.0f;
        
        //Read in image
        result[k] = new View(cvLoadImage(file.c_str()), K, R, S);
    }
    
    return result;
}
