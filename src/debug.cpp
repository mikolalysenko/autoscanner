#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>

#include <Eigen/Core>

#include "debug.h"
#include "system.h"

using namespace std;
using namespace Eigen;

//Saves a collection of point/color pairs to a PLY file for debugging
void savePLY(
    const string& filename, 
    const vector<Vector3d>& points, 
    const vector<Color>& color)
{
    assert(points.size() == color.size());
    
    ofstream fout(filename.c_str(), ios_base::out | ios_base::trunc);

    //Write header
    fout 
        << "ply" << endl
        << "format ascii 1.0" << endl
        << "comment output from autoscanner" << endl
        << "element vertex " << points.size() << endl
        << "property float x" << endl
        << "property float y" << endl
        << "property float z" << endl
        << "property uchar blue" << endl
        << "property uchar green" << endl
        << "property uchar red" << endl
        << "end_header" << endl;

    //Serialize point/color pairs
    for (size_t i = 0; i < points.size(); i++)
        fout << (float)points[i](0) << " " 
             << (float)points[i](1) << " " 
             << (float)points[i](2) << " " 
             << color[i].b  << " " 
             << color[i].g  << " " 
             << color[i].r  << endl;

    fout.close();
}

//Saves a collection of cameras for debugging
void saveCameraPLY(
    const string& filename, 
    const vector<View>& views)
{
    vector<Vector3d> points;
    vector<Color> colors;
    
    for(size_t i=0; i<views.size(); i++)
    {
        points.push_back(views[i].center());
        colors.push_back(Color(rand()%255, rand()%255, rand()%255));
    }
    
    savePLY(filename, points, colors);
}
