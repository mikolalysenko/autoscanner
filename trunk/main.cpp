/**
 * Multiview Stereo
 *
 * Author: Mikola Lysenko
 */

//STL
#include <vector>

//Project files
#include "misc.h"
#include "volume.h"
#include "view.h"
#include "photohull.h"

//Namespace aliasing
using namespace std;

#define SIZE 	200

//Program start point
int main(int argc, char** argv)
{
	//Read in some views
	cout << "reading in files" << endl;

    vec3 low(-0.2f, -0.14f, -0.14f), 
		high( 0.2f, 0.4f,  0.08f);

	vector<View*> views = loadViews("templeR/templeR_par.txt", 
		low,
        high,
		ivec3(SIZE, SIZE, SIZE),
                1.0);

/*
	vector<View*> views = loadViews("temple/temple_par.txt", 
		vec3(-0.12f, -0.1f, -0.04f), 
		vec3( 0.13f, 0.4f,  0.1f),
		ivec3(SIZE, SIZE, SIZE));
*/

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
	Volume * volume = findHull(views, SIZE, SIZE, SIZE, low, high);
	
	cout << "Saving..." << endl;
	volume->save("test/test");
	
	return 0;
}

