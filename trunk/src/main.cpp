//STL
#include <vector>
#include <fstream>

//Eigen
#include <Eigen/Core>

//Project files
#include "view.h"
#include "sfm.h"
#include "debug.h"

using namespace std;



//Program start point
int main(int argc, char** argv)
{
    vector<View> views = parseBundlerTemps("data/lincoln");
    
    saveCameraPLY("out/test.ply", views);
    
    return 0;
}

