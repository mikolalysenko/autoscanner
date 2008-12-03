#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "misc.h"
#include "view.h"

using namespace std;
using namespace blitz;

//Sample every X frames
#define FRAMES_PER_SAMPLE       10

//Path to bundler script
const char* bundler_path = "bundler/RunBundler.sh";

//Camera produced by bundler
struct BundlerCamera
{
    mat44 world;
    float f, k1, k2;
};

//Extract frames from video capture sequence
//TODO: Need to implement a better criteria for this
vector<IplImage*> splitVideo(CvCapture * capture)
{
    vector<IplImage*> frames;
    
    while(true)
    {
        //Skip a fixed number of frames
        for(int i=0; i<FRAMES_PER_SAMPLES; i++)
        {
            if(!cvGrabFrame(capture))
                return frames;
        }
        
        //Retrieve the frame
        IplImage* tmp = cvRetrieveFrame(capture), img;
        img = cvCreateImage(cvSize(tmp->width, tmp->height), IPL_DEPTH_8U, 3);
        cvCopyImage(tmp, img);
        
        //Add to frame set
        frames.push_back(img);
    }
}


//Calls bundler script, calculates bounding box from points found using SfM
vector<BundlerCamera> runBundler(
    vector<IplImage*> frames, 
    vec3& box_min, 
    vec3& box_max)
{
    //Create paths
    string temp_directory = string(getTempDirectory());
    string list_path = temp_directory + "/list.txt";
    string options_path = temp_directory + "/options.txt";
    
    //Open list/options file
    ofstream f_list     = ofstream(list_path),
             f_options  = ofstream(options_path);
    
    
    //Write frames to file
    for(size_t i=0; i<frames.size(); i++)
    {
        
    }
    
    f_list.close();
    f_options.close();
    
    
    //Call bundler
    string bundler_command = string(bundler_path) + " " + list_path + " --options_file " + options_path;
    system(bundler_command.c_str());
    
    //Process results
}


//Undistorts image from k1 - k2
IplImage unwarp(IplImage * img, float k1, float k2)
{
    return img;
}


//Loads up a view from file
vector<View*> loadVideo(const char * filename, ivec3 grid_dim)
{
    //Separate fames
    CvCapture * capture = cvCaptureFromAVI(filename);
    assert(capture);
    vector<IplImage*> frames = splitVideo(capture);
    cvReleaseCapture(capture);
    
    //Run bundler on frames to obtain matrices & camera parameters
    vec3 box_min, box_max;
    vector<BundlerCamera> cameras = runBundler(frames, box_min, box_max);
    
    //Construct grid -> box matrix
    mat44 S = 0.0f;
    for(i=0; i<2; i++)
    {
        S(i,i) = (box_max - box_min) / (float)grid_dim(i);
        S(i,3) = box_min(i);
    }
    S(3,3) = 1.0f;
    
    //Unwarp images & build views
    vector<View*> views;
    
    for(size_t n=0; n<frames.size(); n++)
    {
        //Compute intrinsic matrix K
        mat44 K = 0.0f;
        for(int i=0; i<3; i++)
            K(i,i) = 1.0f;
        K(0,3) = frames[n]->width/2.0f;
        K(1,3) = frames[n]->height/2.0f;
        
        //Construct perspective warping matrix
        mat44 P = 0.0f;
        for(int i=0; i<4; i++)
            P(i,i) = 1.0f;
        P(3,3) = -1.0f;
        P(3,2) = 1.0f / cameras[n].f;
        P(2,2) = 0.0f;
        K = mmult(K, P);
        
        //Undistort cameras
        views.push_back(new View(unwarp(frames[n], cameras[n].k1, cameras[n].k2), K, cameras[n].R, S));
    }
    
    return views;
}