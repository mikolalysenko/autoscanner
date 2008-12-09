#include "kutulakis.h"
#include "config.h"

#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;
using namespace cfg;

config loadKutulakisCameras(const std::string& dir, const std::string& filename, vec3 low, vec3 high, ivec3 box) {
    config files("kutu_cameras"), out("kutu_camera_calib");
    files.load(dir + filename);
    for (config::data_map::iterator iter = files.dataBegin(); iter != files.dataEnd(); iter++) {
        out.set(iter->first, openKutulakisCamera(dir, iter->second, low, high, box));
    }
    return out;
}

config openKutulakisCamera(const std::string& dir, const std::string& filename, vec3 lo, vec3 hi, ivec3 box) {
    config out(dir + filename);
    vec3 center;
    mat44 cam, cam_inv;

    mat44 K, R, S;
    mat44 xRot, yRot, zRot;
    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
        K(i,j) = R(i,j) = S(i,j) = 0;
        xRot(i,j) = yRot(i,j) = zRot(i,j) = 0;
    }
    vec3 euler,image_center, T, sc;
    double focal, scale;

    string line;
    ifstream fin((dir + filename).c_str());
    
    while (getline(fin, line)) {
        stringstream buf(line);        
        cout << "~~  " << line << endl;
        string key;
        buf >> key;
        key = key.substr(0, key.size() - 1);
        cout << key << endl;
        if (key == "rotation_x") {
            buf >> euler(0);
        } else if (key == "rotation_y") {
            buf >> euler(1);
        } else if (key == "rotation_z") {
            buf >> euler(2);
        } else if (key == "translation_x") {
            buf >> T(0);
        } else if (key== "translation_y") {
            buf >> T(1);
        } else if (key == "translation_z") {
            buf >> T(2);
        } else if (key == "focal_length") {
            buf >> focal;
        } else if (key == "image_center_x") {
            buf >> image_center(0);
        } else if (key == "image_center_y") {  
            buf >> image_center(1);      
        } else if (key == "scale_x") {
            buf >> scale;
        } else if (key == "sel_size_x") {
            buf >> sc(0);
        } else if (key == "sel_size_y") {
            buf >> sc(1);
        }
    }

    xRot(1,1) = xRot(2,2) = cos(euler(0));
    xRot(2,1) = - (xRot(1,2) = sin(euler(0)));
    xRot(0,0) = xRot(3,3) = 1;

    yRot(0,0) = yRot(2,2) = cos(euler(1));
    yRot(0,2) = - (yRot(2,0) = sin(euler(1)));
    yRot(1,1) = yRot(3,3) = 1;

    zRot(0,0) = zRot(1,1) = cos(euler(2));
    zRot(1,0) = - (zRot(0,1) = sin(euler(2)));
    zRot(2,2) = zRot(3,3) = 1;

    R = mmult(zRot, mmult(yRot, xRot)); 
    R(3,3) = 1;

    for(int i=0; i<3; i++)
        S(i,i) = (hi(i) - lo(i)) / (float)(box(i) - 1);
    S(3,3) = 1.0f;
    for(int i=0; i<3; i++)
        S(i,3) = lo(i);

    //T = hgmult(S, T);
    T = hgmult(R, hgmult(S, hgmult(transpose(R), T)));
    
    K(0,0) = K(1,1) = focal;
    K(3,2) = 1;
    

    cout << "~~~   K = /n" << K << endl;

    mat44 D, C;
    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
        D(i,j) = 0;

    D(1,1) = - (D(0,0) = D(3,3) = 1);
    D(0,0) /= sc(0);
    D(1,1) /= sc(1);
    D(0,3) = image_center(0);
    D(1,3) = image_center(1);

    
    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
        cam(i,j) /= cam(3,3);

    cam = R;
    cam(0,3) = T(0);
    cam(1,3) = T(1);
    cam(2,3) = T(2);

    cam = mmult(K, cam);

    cam = mmult(D, cam);

    mat44 fix;
    for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
        fix(i,j) = 0;
    
    fix(0,0) = 1;
    fix(1,1) = -1;
    fix(1,3) = 480;
    fix(3,3) = fix(2,2) = 1;
    cam = mmult(fix, cam);

    cout << cam << endl;
    
    center = hgmult(cam_inv, vec3(0, 0, 0));

    cout << center << endl;

    out.set("cam", cam);
    out.set("cam_inv", cam_inv);
    out.set("center", center);
    std::string img_filename(dir + filename.substr(0, filename.size() - 4) + ".tif");
    out.set("img_filename", img_filename);

    return out;
}
