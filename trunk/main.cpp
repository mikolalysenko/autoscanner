/**
 * Multiview Stereo
 *
 * Author: Mikola Lysenko
 */

//STL
#include <vector>
#include <fstream>

//Project files
#include "misc.h"
#include "volume.h"
#include "view.h"
#include "photohull.h"
#include "config.h"
#include "kutulakis.h"

//Namespace aliasing
using namespace std;
using namespace cfg;

//Program start point
int main(int argc, char** argv)
{
    config::global.load("autoscanner.cfg");

    cout << " 0 " << endl;
    cout << config::global.get<std::string>("config_file") << endl;
    config::global.load(config::global.get<std::string>("config_file"));
    
    cout << " 1 " << endl;

    vec3 low, high;
    low = config::global.get("low", low);
    high = config::global.get("high", high);
    int size = config::global.get<int>("volume_resolution");
    
    //kutu = loadKutulakisCameras("gargoyle/", "camerafiles.cfg", low, high, ivec3(SIZE, SIZE, SIZE));
    //kutu.save("kutu.cfg");

	//vector<View*> views = loadTempViews(config::global.get<std::string>("views_file"));

    cout << " here " << endl;
    
	vector<View*> views = loadViews("templeR/templeR_par.txt", 
		low, 
		high,
		ivec3(size, size, size));
    //saveTempViews("templeR/", "temple_cams.cfg", views);


    vec3 bgcolors[] = {
        ivec3(0,255,0),
        ivec3(255,0,0),
        ivec3(0,255,0),
        ivec3(0,0,255),
        ivec3(255,255,255)};

    for (int i = 0; i < views.size(); i++) {
        vec3 bgcolor = bgcolors[i % 5];
        
        for (int x = 0; x < views[i]->img->width; x++)
        for (int y = 0; y < views[i]->img->height; y++) {
            vec3 pix = views[i]->readPixel(x,y);
            if (pix(0) + pix(1) + pix(2) < 15) {
                for (int t = 0; t < 3; t++)
                    views[i]->img->imageData[3 * (x + y * views[i]->img->width) + t ] = bgcolor[t];
            }

        }

        stringstream buf;
        buf << "temp/testimg" << i << ".png";
        cvSaveImage(buf.str().c_str(), views[i]->img);
        
    }
    

	/*
	vector<View*> views = loadViews("dino/dino_par_good.txt",
		vec3(-0.00, -0.01,  -0.05),
		vec3( 0.05,  0.1,    0.05),
		ivec3(SIZE, SIZE, SIZE));
	*/
	
	/*
	cout << "Testing camera 0: " << endl 
		 << hgmult(views[0]->cam, vec3(0, 0, 0)) << endl
		 << hgmult(views[0]->cam, vec3(SIZE, 0, 0)) << endl
		 << hgmult(views[0]->cam, vec3(0, SIZE, 0)) << endl
		 << hgmult(views[0]->cam, vec3(0, 0, SIZE)) << endl
		 << hgmult(views[0]->cam, vec3(SIZE, SIZE, 0)) << endl
		 << hgmult(views[0]->cam, vec3(SIZE, 0, SIZE)) << endl
		 << hgmult(views[0]->cam, vec3(0, SIZE, SIZE)) << endl
		 << hgmult(views[0]->cam, vec3(SIZE, SIZE, SIZE)) << endl;
    */
	
	

    
	cout << "Finding photo hull" << endl;
	Volume * volume = findHull(views, size, size, size, low, high);
	
	cout << "Saving..." << endl;
    volume->save("test/test");
    
    for (size_t i=0; i<views.size(); i++) {
        string fname = "test/consist"; fname += '0' + i; fname += ".png";
        views[i]->writeConsist(fname);
    }

	
	return 0;
}

