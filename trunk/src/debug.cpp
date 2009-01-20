#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>

#include <Eigen/Core>

#include "debug.h"
#include "system.h"

using namespace std;
using namespace Eigen;

//Saves a collection of point/color pairs to a PLY file for debugging
void savePLY(
    const string& filename, 
    const vector<Vector3d>& points, 
    const vector<Vector3i>& color)
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
             << (ubyte)color[i](0)  << " " 
             << (ubyte)color[i](1)  << " " 
             << (ubyte)color[i](2)  << endl;

    fout.close();
}

//Saves a collection of cameras for debugging
void saveCameraPLY(
    const string& filename, 
    const vector<View>& views)
{
    vector<Vector3d> points;
    vector<Vector3i> colors;
    
    for(size_t i=0; i<views.size(); i++)
    {
        points.push_back(views[i].center());
        colors.push_back(Vector3i(rand()%255, rand()%255, rand()%255));
    }
    
    savePLY(filename, points, colors);
}
