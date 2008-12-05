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
#define FRAMES_PER_SAMPLE       30

//Path to bundler script
const char* bundler_path = "/home/mikola/Projects/autoscanner/bundler/RunBundler.sh";

//Camera produced by bundler
struct BundlerCamera
{
    mat44 R;
    float f, k1, k2;
};

//Used for debugging
ostream& operator<<(ostream& os,  const BundlerCamera& cam)
{
    return os 
        << "{f=" << cam.f 
        << ", k1=" << cam.k1
        << ", k2=" << cam.k2 
        << ", R=" << cam.R 
        << "}";
}

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

//Reads in bundler data
vector<BundlerCamera> readBundlerData(
    const char * filename, 
    vec3& box_min, 
    vec3& box_max)
{
    cout << "Reading in bundler data:" << filename << endl;
    
    //Create read file
    ifstream fin(filename);
    
    //Skip first line (should be "#Bundle file v0.3")
    static char buf[1024];
    fin.getline(buf, 1024);
    assert(strcmp(buf, "# Bundle file v0.3") == 0);
    
    
    //Read in number of data elements
    int n_cameras, n_points;
    fin >> n_cameras >> n_points;
    
    cout << "Num Cameras = " << n_cameras << ", Num points = " << n_points << endl;
    
    //Parse out cameras first
    vector<BundlerCamera> result;
    
    for(int k=0; k<n_cameras; k++)
    {
        cout << "Reading camera #" << k << endl;
        
        BundlerCamera cam;
        fin >> cam.f >> cam.k1 >> cam.k2;
        
        for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
        {
            double d;
            assert(fin >> d);
            cout << d << endl;
            
            cam.R(i,j) = d;
        }
        
        for(int i=0; i<3; i++)
        {
            double d;
            assert(fin >> d);
            cout << d << endl;
            
            cam.R(i,3) = d;
            cam.R(3,i) = 0.0f;
        }
        
        cam.R(3,3) = 1.0f;

        cout << "Camera " << k << " = " << cam << endl;
        result.push_back(cam);
    }
    
    
    
    for(int i=0;;i++)
    {
        string s;
        assert(fin >> s);
        cout << i << "Got: " << s << endl;
    }
    
    

    
    //Parse out point data & calculate bounding box
    box_min = 1e30, box_max = -1e30;
    
    cout << "Reading point data..." << endl;

    
    
    for(int i=0; i<n_points; i++)
    {
        vec3 p, c;
        
        cout << "Reading point " << i << "..." << endl;
        
        fin >> p(0);
        fin >> p(1);
        fin >> p(2);

        cout << "got p: " << p << endl;
        
        if(!(fin >> p(0) >> p(1) >> p(2) >> c(0) >> c(1) >> c(2)))
        {
            cout << "Unexpected EOF!" << endl;
            break;
        }
        
        cout << "got p: " << p << endl;
        cout << "color = " << c << endl;
        
        cout << "Visible in: ";
        
        //Read in visible locations
        int n_views;
        fin >> n_views;
        for(int j=0; j<n_views; j++)
        {
            int v, k;           //Camera, feature point
            float x, y;         //x-y pixel coordinates
            
            fin >> v >> k >> x >> y;
            cout << "(" << v 
                << ", " << k 
                << ", " << x 
                << ", " << y 
                << ") ";
        }
        cout << endl;
        
        //Update box coordinates
        for(int j=0; j<3; j++)
        {
            box_min(j) = min(box_min(j), p(j));
            box_max(j) = max(box_max(j), p(j));
        }
    }
    
    cout << "Box = " << box_min << " --- " << box_max << endl;
    
    if(abs(box_min(0) - box_max(0)) <= 1e-10 ||
        abs(box_min(1) - box_max(1)) <= 1e-10 ||
        abs(box_min(2) - box_max(2)) <= 1e-10)
    {
        cout << "Couldn't compute box bounds" << endl;
        box_min = vec3(-1,-1,-1);
        box_max = vec3(1,1,1);
    }
    
    //Return the result
    return result;
}


//Toss the bad bundler data
void fixupBundlerData(vector<IplImage*>& frames, vector<BundlerCamera>& cameras)
{
    int n_failures = 0;
    cout << "Filtering camera data..." << endl;
    
    for(int i=(int)cameras.size() - 1; i>=0; i--)
    {
        float d = det(cameras[i].R);
        
        //Check for singular camera matrix (obviously bad)
        if(abs(d) <= 1e-10f)
        {
            cout << "Singular matrix for camera " << i << ", D = " << d << endl
                << "cam = " << cameras[i] << endl;
            
            //Release iamge
            cvReleaseImage(&frames[i]);
            
            //Resize vectors
            cameras[i] = cameras[cameras.size()-1];
            frames[i] = frames[frames.size()-1];
            cameras.resize(cameras.size()-1);
            frames.resize(frames.size()-1);
            
            n_failures ++;
        }
    }
    
    cout << n_failures << " failures" << endl;
    if(n_failures > 0)
        cout << "Way to go bundler..." << endl;
}


//Calls bundler script, calculates bounding box from points found using SfM
vector<BundlerCamera> runBundler(
    vector<IplImage*>& frames, 
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
        char file_name[1024];
        snprintf(file_name, 1024, "%s/frame%04d.jpg",  temp_directory.c_str(), i);
        
        cout << "Saving frame: " << file_name << endl;
        cvSaveImage(file_name, frames[i]);
    }
    
    //Call bundler
    string bundler_command = string(bundler_path) + " " + temp_directory;
    system(bundler_command.c_str());
    
    //Read in data (do not know format yet)
    vector<BundlerCamera> result = readBundlerData(
        (temp_directory + "/bundle/bundle.out").c_str(),
        box_min,
        box_max);
    
    //Toss bad data
    fixupBundlerData(frames, result);
    
    //Return to base directory
    chdir(cur_directory.c_str());
    
    return result;
}


//Undistorts image from k1 - k2
IplImage * unwarp(IplImage * img, float k1, float k2)
{
    /*
    //Interface for doing this has changed...  Need to work out details
    
    //Allocate temp buffer
    int * data = (int*)malloc(3 * img->width * img->height);
    
    //matrices
    float matrix[9];
    for(int i=0; i<9; i++)
        matrix[i] = 0.0f;
    matrix[0] = matrix[4] = matrix[7] = 1.0f;
    
    float coeff[4];
    coeff[0] = k1;
    coeff[1] = k2;
    coeff[2] = coeff[3] = 0.0f;
    
    IplImage * undist = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
    
    //Undistort image
    cvUndistortInitMap(img, undist, matrix, coeff, data, true);
    
    //For debugging, save undistorted image
    cvUndistortOnce(img, undist, data, true);
    cvSaveImage("undistort.jpg", undist);
    cvCopyImage(undist, img);
    cvReleaseImage(undist);
    
    //Release temporary data
    free(data);
    */
    
    //Not yet implemented.  Need to read OpenCV documentation
    return img;
}


//Converts bundler formatted data + pictures to camera data
vector<View*> convertBundlerData(
    vector<IplImage*> frames,
    vector<BundlerCamera> cameras,
    ivec3 grid_dim,
    vec3 box_min,
    vec3 box_max)
{
    //Construct grid -> box matrix
    mat44 S;
    for(int i=0; i<4; i++)
    for(int j=0; j<4; j++)
        S(i,j) = 0.0f;
    for(int i=0; i<4; i++)
        S(i,i) = 1.0f;
    
    for(int i=0; i<2; i++)
    {
        S(i,i) = (box_max(i) - box_min(i)) / (float)grid_dim(i);
        S(i,3) = box_min(i);
    }
    S(3,3) = 1.0f;
    
    cout << "S matrix = " << S << endl;
    
    //Unwarp images & build views
    vector<View*> views;
    
    for(size_t n=0; n<frames.size(); n++)
    {
        cout << "Processing frame " << n << endl;
        
        //Compute intrinsic matrix K
        mat44 K;
        for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            K(i,j) = 0.0f;
        for(int i=0; i<4; i++)
            K(i,i) = 1.0f;
        K(0,3) = frames[n]->width/2.0f;
        K(1,3) = frames[n]->height/2.0f;
        
        /*
        //Construct perspective warping matrix
        mat44 P;
        for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            P(i,j) = 0.0f;
        for(int i=0; i<4; i++)
            P(i,i) = 1.0f;
        P(3,3) = -1.0f;
        P(3,2) = 1.0f;
        K = mmult(K, P);
        */
        
        cout << "K = " << K << endl;
        
        //Undistort cameras
        views.push_back(new View(
            unwarp(frames[n], cameras[n].k1, cameras[n].k2), 
            K, 
            cameras[n].R, 
            S));
    }
    
    return views;
}


//Loads up a view from file
vector<View*> loadVideo(const char * filename, ivec3 grid_dim, vec3& box_min, vec3& box_max)
{
    //Separate fames
    vector<IplImage*> frames = splitVideo(filename);
    
    //Run bundler on frames to obtain matrices & camera parameters
    vector<BundlerCamera> cameras = runBundler(frames, box_min, box_max);
    
    //Convert data to internal view data structure
    return convertBundlerData(frames, cameras, grid_dim, box_min, box_max);
}


//Loads the intermediate bundler data from temporary storage
//Needed for testing
vector<View*> loadTempBundleData(
    const char* directory, 
    ivec3 grid_dim, 
    vec3& box_min, 
    vec3& box_max)
{
    //Load up images from parsed list file
    ifstream fin((string(directory) + "/list.txt").c_str());
    vector<IplImage*> frames;
    
    while(true)
    {
        string name;
        if(!(fin >> name))
            break;
        
        //Trim off prefix directory
        for(int k=name.size()-1; k>=0; k--)
        {
            if(name[k] == '/')
            {
                name.erase(0, k);
                break;
            }
        }
        name = string(directory) + string("/") + name;
        
        cout << "Loading image " << name << endl;
        
        IplImage * img = cvLoadImage(name.c_str());
        frames.push_back(img);
    }
    
    //Load up the cameras
    vector<BundlerCamera> cameras = readBundlerData(
        (string(directory) + "/bundle/bundle.out").c_str(),
        box_min, box_max);
    
    //Fix up bundler data
    fixupBundlerData(frames, cameras);
    
    //Convert data to internal format and return
    return convertBundlerData(frames, cameras, grid_dim, box_min, box_max);
}
