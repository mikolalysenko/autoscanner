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

//Namespace aliasing
using namespace std;

#define SIZE 	128

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
    
        vec3 low, high;
        //vector<View*> frames = loadVideo("mvi_0876.avi", ivec3(SIZE, SIZE, SIZE), low, high);
        
        vector<View*> frames = loadTempBundleData(
            "test_bundle2",
            ivec3(SIZE, SIZE, SIZE),
            low, high);
        
        saveTempViews("temp_views", frames);
        
        //vector<View*> frames = loadTempViews("temp_views");
        
        Volume * hull = findHull(frames, 
            SIZE,SIZE,SIZE,
            low, high);
        
        hull->save("test/test");
    
        
	return 0;
}

