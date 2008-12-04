#include <vector>
#include <sstream>
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
const char* bundler_path = "/home/mikola/Projects/autoscanner/bundler/RunBundler.sh";

//Camera produced by bundler
struct BundlerCamera
{
    mat44 R;
    float f, k1, k2;
};

//Extract frames from video capture sequence
//TODO: Need to implement a better criteria for this
vector<IplImage*> splitVideo(const char * filename)
{
    cout << "Loading image " << filename << endl;
    
    //Run the capture process
    CvCapture * capture = cvCaptureFromAVI(filename);
    assert(capture);
    
    vector<IplImage*> frames;
    
    while(true)
    {
        //Skip a fixed number of frames
        for(int i=0; i<FRAMES_PER_SAMPLE; i++)
        {
            if(!cvGrabFrame(capture))
                return frames;
        }
        
        //Retrieve the frame
        IplImage *tmp = cvRetrieveFrame(capture), *img;
        img = cvCreateImage(cvSize(tmp->width, tmp->height), IPL_DEPTH_8U, 3);
        cvCopyImage(tmp, img);
        
        //Add to frame set
        frames.push_back(img);
    }
    
    cvReleaseCapture(&capture);
}


//Calls bundler script, calculates bounding box from points found using SfM
vector<BundlerCamera> runBundler(
    vector<IplImage*> frames, 
    vec3& box_min, 
    vec3& box_max)
{
    //Create paths
    string temp_directory = string(getTempDirectory()) + "/bundler";
    string cur_directory = string(getenv("PWD"));
    system((string("rm -rf ") + temp_directory).c_str());
    system((string("mkdir ") + temp_directory).c_str());
    chdir(temp_directory.c_str());
    
    //Write frames to file
    for(size_t i=0; i<frames.size(); i++)
    {
        stringstream ss;
        ss << temp_directory << "/frame" << i << ".jpg";
        
        cout << "Saving frame: " << ss.str() << endl;
        cvSaveImage(ss.str().c_str(), frames[i]);
    }
    
    //Call bundler
    string bundler_command = string(bundler_path) + " " + temp_directory;
    system(bundler_command.c_str());
    
    //Read in data
    ifstream bundle_data((temp_directory + "/bundle_.out").c_str());
    vector<BundlerCamera> result;
    for(size_t i=0; i<frames.size(); i++)
    {
        
    }

    //Return to base directory
    chdir(cur_directory.c_str());
    
    return result;
}


//Undistorts image from k1 - k2
IplImage * unwarp(IplImage * img, float k1, float k2)
{
    return img;
}



//Loads up a view from file
vector<View*> loadVideo(const char * filename, ivec3 grid_dim)
{
    //Separate fames
    vector<IplImage*> frames = splitVideo(filename);
    
    //Run bundler on frames to obtain matrices & camera parameters
    vec3 box_min, box_max;
    vector<BundlerCamera> cameras = runBundler(frames, box_min, box_max);
    
    //Construct grid -> box matrix
    mat44 S;
    
    for(int i=0; i<4; i++)
    for(int j=0; j<4; j++)
        S(i,j) = 0.0f;
    
    for(int i=0; i<2; i++)
    {
        S(i,i) = (box_max(i) - box_min(i)) / (float)grid_dim(i);
        S(i,3) = box_min(i);
    }
    S(3,3) = 1.0f;
    
    //Unwarp images & build views
    vector<View*> views;
    
    for(size_t n=0; n<frames.size(); n++)
    {
        //Compute intrinsic matrix K
        mat44 K;
        for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            K(i,j) = 0.0f;
        for(int i=0; i<3; i++)
            K(i,i) = 1.0f;
        K(0,3) = frames[n]->width/2.0f;
        K(1,3) = frames[n]->height/2.0f;
        
        //Construct perspective warping matrix
        mat44 P;
        for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            P(i,j) = 0.0f;
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
