#include <iostream>
#include <string>
#include <fstream>

#include <Eigen/Core>

USING_PART_OF_NAMESPACE_EIGEN

using namespace std;

//Locates the temp directory
char * getTempDirectory()
{
    if(getenv("TMPDIR"))
        return getenv("TMPDIR");
    if(getenv("TEMP"))
        return getenv("TEMP");
    if(getenv("TMP"))
        return getenv("TMP");
    return "/tmp/";
}

//Saves a collection of point/color pairs to a PLY file for debugging
void savePly(
    const string& filename, 
    const vector<Vector3f>& points, 
    const vector<Vector3i>& color)
{
    typedef const vector<Vector3f> pointVector;
    typedef const vector<Vector3i> colorVector;

    assert(points.size() == color.size());
    
    ofstream fout(filename.c_str(), ios_base::out | ios_base::trunc);

    fout << "ply" << endl;
    fout << "format ascii 1.0" << endl;
    fout << "comment output from autoscanner" << endl;
    fout << "element vertex " << points.size() << endl;
    fout << "property float x" << endl;
    fout << "property float y" << endl;
    fout << "property float z" << endl;
    fout << "property uchar blue" << endl;
    fout << "property uchar green" << endl;
    fout << "property uchar red" << endl;
    fout << "end_header" << endl;

    for (int i = 0; i < points.size(); i++)
        fout << points[i](0) << " " << points[i](1) << " " << points[i](2) << " " << color[i](0) << " " << color[i](1) << " " << color[i](2) << endl;

    fout.close();
}
