#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "misc.h"
#include "view.h"

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
using namespace blitz;

#define FILL_THRESHOLD          60

//Does some thresholding on the input image
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


void fill_rec(
    IplImage * img, 
    ivec3 fill_c,
    ivec3 base_c,
    int x, int y)
{
    //cout << " fill_rec " << x << " " << y << endl;
    if(x < 0 || x >= img->width || y < 0 || y >= img->height)
        return;
    
    int idx = 3 * (x + y * img->width);
    
    bool eq = true;
    ivec3 c;
    
    //cout << " 1 " << endl;
    for(int i=0; i<3; i++)
    {
        c(i) = (unsigned char)img->imageData[idx + i];
        eq &= c(i) == fill_c(i);
    }
    //cout << " 2 " << endl;
    
    if(eq) return;
    
    c -= base_c;
    
    //cout << " 3 " << endl;
    if(len(c) <= FILL_THRESHOLD)
    {
        for(int i=0; i<3; i++)
            ((unsigned char*)img->imageData)[idx + i] = fill_c(i);
        
        for(int dx=-2; dx<=2; dx++)
        for(int dy=-2; dy<=2; dy++)
        {
            fill_rec(img, fill_c, base_c, x+dx, y+dy);
        }
    }
    //cout << " 4 " << endl;
}


void fill_hack(IplImage * img)
{
    ivec3 fill_c = ivec3(rand() % 256, rand() % 256, rand() % 256);
    
    for(int x=0; x<img->width; x++)
        fill_rec(img, 
            fill_c,
            ivec3((unsigned char)img->imageData[3*x], 
                (unsigned char)img->imageData[3*x + 1], 
                (unsigned char)img->imageData[3*x + 2]),
            x, 0);
}


void View::init(mat44 K_, mat44 R_, mat44 S_) {
    K = K_; R = R_; S = S_;
    cout << " copied " << endl;
    //Calculate camera matrices
    cam = mmult(K, mmult(R, S));
    cout << " 1 " << endl;

    //Needed for view saving
    cam_inv = inverse(mmult(R, S));
    cout << " 2 " << endl;
    
    //Compute optical center
    center = hgmult(inverse(mmult(R, S)), vec3(0, 0, 0));
    cout << " 3 " << endl;

    //Debug spam
    cout << "Camera: " << cam << endl
         << "K = " << K << endl
         << "R = " << R << endl
         << "S = " << S << endl
         << "CamInv: " << cam_inv << endl
         << "Center = " << center << endl;
}

//View constructor
View::View(IplImage * pic, mat44 cam_, mat44 cam_inv_, vec3 center_)
{
    assert(img != NULL);
    
    //Apply filters (this is very tweaky)
    img = pic;
    
    fill_hack(img);
    
    //Create data
    consist_data = (char*)malloc(img->width * img->height);
    
    //Calculate camera matrices
    cam = cam_;
    cam_inv = cam_inv_;
    center = center_;

    //Debug spam
    cout << "Camera: " << cam << endl
         << "CamInv: " << cam_inv << endl
         << "Center = " << center << endl;

}

//View constructor
View::View(IplImage * pic, mat44 K_, mat44 R_, mat44 S_)
{
    assert(img != NULL);
    
    img = pic;
    
    cout << " 1 " << endl;
    fill_hack(pic);
    cout << " 2 ~~~ " << endl;
    
    //Create data
    consist_data = (char*)malloc(img->width * img->height);

    init(K_,R_,S_);   
    
}


//Read a pixel from the view
vec3 View::readPixel(int ix, int iy) const
{
    //Clamp coordinates
    ix = min(max(ix, 0), img->width - 1);
    iy = min(max(iy, 0), img->height - 1);
    
    //Read from image
    unsigned char * ptr = reinterpret_cast<unsigned char*>(&img->imageData[3 * (ix ) + 3 * iy * img->width]);
    
    //Convert pixel coordinates
    return vec3(ptr[0], ptr[1], ptr[2]);
}


//Loads up a set of views
vector<View*> loadViews(const char * filename, vec3 lo, vec3 hi, ivec3 box, float focal_length)
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
        
        //Load image
        string file;
        fin >> file;
        IplImage * img = cvLoadImage(file.c_str());
        
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

        
        //Apply focal distance matrix
        mat44 focal;
        for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            focal(i,j) = 0.0f;
        for(int i=0; i<4; i++)
            focal(i,i) = 1.0f;
            
        //Set value to focal length
        focal(3,2) = 1.0f / focal_length;
        K = mmult(K, focal);
        
        //Read in image
        result[k] = new View(img, K, R, S);
    }
    
    return result;
}

void View::writeConsist(const std::string& filename) {

    IplImage * consistImg = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
    
    cvCopyImage(img, consistImg);
    
    unsigned char* img_ptr = ((unsigned char*)consistImg->imageData) + 2;
    unsigned char* consist_ptr = (unsigned char*)consist_data;
    
    for(int i=0; i<img->width; i++)
    for(int j=0; j<img->height; j++)
    {
        if(*(consist_ptr++))
            *img_ptr = 0xff;
        img_ptr+=3;
    }
    
    
    cvSaveImage(filename.c_str(), consistImg);
    cvReleaseImage(&consistImg);
}

config View::save(const std::string& name, const std::string& dir) {
    config data(name);
    data.set("cam", cam);
    data.set("cam_inv", cam_inv);
    data.set("center", center);
    std::string img_filename(dir + name + ".tif");
    data.set("img_filename", img_filename);
    cvSaveImage(img_filename.c_str(), img);
    data.set("K", K);
    data.set("R", R);
    return data;
}

void View::load(config& data) {
    mat44 K_, R_, S_;

    cout << "loading matrices :/" << endl;
    K_ = data.get("K", K_);
    R_ = data.get("R", R_);


    vec3 lo, hi; ivec3 box;
    int size = config::global.get<int>("volume_resolution");
    box = size;
    
    lo = config::global.get<vec3>("low");
    hi = config::global.get<vec3>("high");
    for(int i=0; i<4; i++)
    for(int j=0; j<4; j++)
        S_(i,j) = 0;
    for(int i=0; i<3; i++)
        S_(i,i) = (hi(i) - lo(i)) / (float)(box(i) - 1);
    S_(3,3) = 1.0f;
    for(int i=0; i<3; i++)
        S_(i,3) = lo(i);
    
    cout << "initing" << endl;
    init(K_,R_,S_);   
    
    
    std::string filename = data.get("img_filename", filename);
    cout << filename << endl;
    img = cvLoadImage(filename.c_str());    
    consist_data = (char*)malloc(img->width * img->height);
}


void saveTempViews(const std::string& directory, const std::string& filename, std::vector<View*> views) {
    config viewsData("Views");

    for (size_t i = 0; i < views.size(); i++) {
        char buf[1024];
        snprintf(buf, 1024, "Camera%04d", i);
        viewsData.set(buf, views[i]->save(buf, directory));        

    }

    viewsData.save(directory + filename);
}

View::View(config& cfg) {
    load(cfg);
}

std::vector<View*> loadTempViews(const std::string& filename) {    
    cout << filename << " ---- " << endl;
    config viewsData("Views");
    viewsData.load(filename);
    cout << " --- " << endl;
    std::vector<View*> out;
    for (config::child_map::iterator iter = viewsData.childBegin(); 
            iter != viewsData.childEnd(); iter++) {
        out.push_back(new View(iter->second));
    }
    return out;
}

void saveCameraPLY(const char * file, vector<View*> views)
{
    std::string filename(file);

    vector<vec3> points;

    for(size_t i=0; i<views.size(); i++)
    {
        points.push_back(views[i]->center);
        
        for(float t=0.0f; t<=1.0; t+=0.1)
        {
            vec3 p = hgmult(views[i]->cam_inv, vec3(0,0,t));            
            points.push_back(p);
        }
    }
    
    //Save points to PLY file
    //savePly(filename, points);
}

