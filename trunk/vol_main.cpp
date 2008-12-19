/**
 * Multiview Stereo
 *
 * Author: Mikola Lysenko
 */

//STL
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

//Project files
#include "misc.h"
#include "volume.h"
#include "view.h"
#include "photohull.h"
#include "volume_cuts.h"
#include "video_loader.h"
#include "config.h"
#include "kutulakis.h"

//Namespace aliasing
using namespace std;

//#define SIZE 	128

//Program start point
int main(int argc, char** argv)
{
    /*
	//Read in some views
	cout << "reading in files" << endl;

	vector<View*> views = loadViews("temple/templeSR_par.txt", 
		vec3(-0.12f, -0.1f, -0.04f), 
		vec3( 0.13f, 0.36f,  0.1f),
		ivec3(SIZE, SIZE, SIZE),
                100.0f);

	//Allocate volumetric graph cuts shape
        Volume * hull = new Volume(SIZE, SIZE, SIZE);
        
        for(int i=0; i<SIZE; i++)
        for(int j=0; j<SIZE; j++)
        for(int k=0; k<SIZE; k++)
        {
            (*hull)(i,j,k) = 255;
        }
        
        //Try solving volume using graph cuts
        Volume * volume = volumetricGraphCuts(
            views,
            vec3(5, 5, 5),
            hull);
        
	cout << "Saving..." << endl;
	volume->save("test/test");
    */
    
        config::global.load("autoscanner.cfg");
        config::global.load(config::global.get<std::string>("config_file"));

        vec3 low, high;
        low = config::global.get("low", low);
        high = config::global.get("high", high);
        
        int size = config::global.get<int>("volume_resolution");
        
        //vector<View*> frames = loadVideo("mvi_0883.avi", ivec3(size, size, size), low, high);
        
        vector<View*> frames = loadTempBundleData(
            "sai3",
            ivec3(size, size, size),
            low, high);
            
            
        for(int i=frames.size()-1; i>=0; i--)
        {
            if(frames[i]->center(0) > 0)
                frames.erase(frames.begin()+i, frames.begin()+i+1);
        }
        
        
        /*
        vector<View*> frames = loadTempBundleData(
            "data/sai2",
            ivec3(size, size, size),
            low, high);
        
        frames.erase(frames.begin() + 33, frames.end());
        frames.erase(frames.begin(), frames.begin() + 3);
        */
        
        /*
        //Remove bad frames
        frames.erase(frames.begin() + 8, frames.begin() + 16);
        frames.erase(frames.begin(), frames.begin() + 2);
        */
        
        //saveTempViews("temp_views/", "list", frames);
        
        cout << "low = " << low << endl
             << "high = " << high << endl;
             
        //Save camera positions
        //saveCameraPLY("cameras.ply", frames);
        
        Volume * hull = findHull(frames, 
            size,size,size,
            low, high);
    
        hull->save("test/test");
        hull->savePly("test/test", frames);
        
	return 0;
}
