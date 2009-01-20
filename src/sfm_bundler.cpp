//stdlib includes
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>

//Eigen
#include <Eigen/Core>
#include <Eigen/LU>

//Project
#include "image.h"
#include "view.h"
#include "system.h"

using namespace std;
using namespace Eigen;

//Debugging stuff, needed to check for camera parameter correctness
struct PointImage
{
    Vector2d loc;
    int camera, key;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

//Camera produced by bundler
struct BundlerCamera
{
    Matrix4d R;
    double f, k1, k2;
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


//Debugging data, used for double checking computed matrix
vector< Vector3f >               points;
vector< vector< PointImage* > >  point_images;

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

//Reads in bundler data
vector<BundlerCamera*> readBundlerData(const string& filename)
{
    cout << "Reading in bundler data:" << filename << endl;
    
    //Create read file
    ifstream fin(filename.c_str());
    
    //Skip first line (should be "#Bundle file v0.3")
    static char buf[1024];
    fin.getline(buf, 1024);
    assert(strcmp(buf, "# Bundle file v0.3") == 0);
    
    
    //Read in number of data elements
    int n_cameras, n_points;
    fin >> n_cameras >> n_points;
    
    cout << "Num Cameras = " << n_cameras 
         << ", Num points = " << n_points << endl;
    
    //Parse out cameras first
    vector<BundlerCamera*> result;
    
    for(int k=0; k<n_cameras; k++)
    {
        assert(!fin.fail());
        
        BundlerCamera* cam = new BundlerCamera();
        fin >> cam->f >> cam->k1 >> cam->k2;
        
        //Reset matrix
        cam->R.setZero();
        
        for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
        {
            fin >> cam->R(i,j);
        }
        
        for(int i=0; i<3; i++)
        {
            fin >> cam->R(i,3);
        }
        
        cam->R(3,3) = 1.0;
        
        cout << "Camera " << k << " = " << *cam << endl;
        result.push_back(cam);
    }
        
    //Parse out point data & calculate bounding box
    points.clear();
    point_images.clear();
    
    cout << "Reading point data..." << endl;
    for(int i=0; i<n_points; i++)
    {
        Vector3d p, c;
        
        if(!(fin >> p.x() >> p.y() >> p.z()
                 >> c.x() >> c.y() >> c.z()))
        {
            cout << "Unexpected EOF" << endl;
            exit(1);
        }
        
        //Read in visible locations
        int n_views;
        fin >> n_views;
        
        vector<PointImage*> p_images;
        for(int j=0; j<n_views; j++)
        {
            PointImage* pi = new PointImage();
            fin >> pi->camera >> pi->key >> pi->loc.x() >> pi->loc.y();
            p_images.push_back(pi);
        }
        points.push_back(p);
        point_images.push_back(p_images);
    }
    
    //Return the result
    return result;
}

//Calls bundler script
vector<BundlerCamera*> runBundler(
    vector<Image> frames, 
    const string& bundler_path)
{
    //Create temp directory
    string temp_directory = getTempDirectory() + "/bundler";
    system((string("rm -rf ") + temp_directory).c_str());
    system((string("mkdir ") + temp_directory).c_str());
    
    //Set current directory to temp directory
    string cur_directory = string(getenv("PWD"));
    chdir(temp_directory.c_str());
    
    //Write frames to file
    for(size_t i=0; i<frames.size(); i++)
    {
        char file_name[1024];
        snprintf(file_name, 1024, "%s/frame%04d.jpg",  temp_directory.c_str(), i);
        
        cout << "Saving frame: " << file_name << endl;
        frames[i].save(file_name);
    }
    
    //Call bundler
    string bundler_command = bundler_path + " " + temp_directory;
    system(bundler_command.c_str());
    
    //Read in data from bundler
    vector<BundlerCamera*> result = 
        readBundlerData(temp_directory + "/bundle/bundle.out");
    
    //Return to base directory
    chdir(cur_directory.c_str());
    
    return result;
}



//Converts bundler formatted data + pictures to camera data
vector<View> convertBundlerData(
    vector<Image> frames,
    vector<BundlerCamera*> cameras)
{
    //Unwarp images & build views
    vector<View> views;
    
    for(size_t n=0; n<frames.size(); n++)
    {
        cout << "Processing frame " << n << endl;
        
        
        //Check for singular rotation matrix
        float d = cameras[n]->R.determinant();
        
        //Check for singular camera matrix (obviously bad)
        if(abs(d) <= 1e-10f)
        {
            cout << "Singular matrix for camera " << n
                 << ", D = " << d << endl
                 << "cam = " << *(cameras[n]) << endl;
            
            continue;
        }
        
        //Construct intrinsic matrix K
        Matrix4d 
            K = Matrix4d::Identity(),
            P = Matrix4d::Zero();
        
        K(1,1) = -1.0;
        K(0,3) = (double)frames[n].width()  / 2.0;
        K(1,3) = (double)frames[n].height() / 2.0;
        
        P(0,0) = -cameras[n]->f;
        P(1,1) = -cameras[n]->f;
        P(2,3) = 1.0f;
        P(3,2) = 1.0f;
        
        //Add view
        views.push_back(View(frames[n], cameras[n]->R, K * P));
            
        /*
        //Check point images
        for(int i=0; i<point_images.size(); i++)
        for(int j=0; j<point_images[i].size(); j++)
        {
            PointImage pi = point_images[i][j];
            
            if(pi.camera == n)
            {
                cout << "p: " << points[i] << endl
                     << "p': " << hgmult(mmult(K, cameras[n].R), points[i]) << endl
                     << "correct: " << pi.x << ", " << pi.y << endl;
            }
        }
        */
    }
    
    //Release bundler cameras
    for(size_t i=0; i<cameras.size(); i++)
        delete cameras[i];
    
    //Release point data
    assert(points.size() == point_images.size());
    
    for(size_t i=0; i<point_images.size(); i++)
    for(size_t j=0; j<point_images[i].size(); j++)
        delete point_images[i][j];
    
    return views;
}



//Runs bundler to solve structure from motion on an unordered collection
//of images
vector<View>  bundlerSfM(
    vector<Image> frames, 
    const string& bundler_path)
{
    return convertBundlerData(
        frames, 
        runBundler(frames, bundler_path));
}

//Loads the intermediate bundler data from temporary storage
//Used for debugging
vector<View> parseBundlerTemps(const std::string& directory)
{
    //Use image paths from bundler's list.txt file
    ifstream fin((string(directory) + "/list.txt").c_str());
    vector<Image> frames;
    
    
    char buffer[1024];
    while(true)
    {
        if(!fin.getline(buffer, 1024))
            break;
        
        char * str = buffer;
        
        //Truncate to string between slash and space
        for(char * ptr = buffer; *ptr; ptr++)
        {
            if(*ptr == '/')
                str = ptr+1;
            else if(*ptr == ' ')
            {
                *ptr = 0;
                break;
            }
        }
        
        string name = directory + "/" + str;
        
        cout << "Loading image " << name << endl;
        
        frames.push_back(Image(name));
    }
    
    //Convert data to internal format and return
    return convertBundlerData(frames, 
        readBundlerData(directory + "/bundle/bundle.out"));
}
