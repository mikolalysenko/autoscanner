#ifndef VIEW_H
#define VIEW_H

#include "config.h"

#include <cassert>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>

#include <Eigen/Core>

using namespace cfg;

//A camera view
struct View
{
    typedef Eigen::Matrix4f mat44;
    typedef Eigen::Vector3f vec3;
    
    //Image data
    IplImage * img;
    
    //Consistency data
    char * consist_data;
    
    //Camera calibration data
    mat44 cam, cam_inv;

    mat44 K, R, S;
    
    //Camera center
    vec3 center;
    
    //Constructs a view
    View(IplImage *img_, mat44 cam_, mat44 cam_inv_, vec3 center_);
    View(IplImage *img_, mat44 K_, mat44 R_, mat44 S_);
    View(config& cfg);
    
    //Reads
    vec3 readPixel(int ix, int iy) const;

    bool in_bounds(int ix, int iy) const {
        return ix >= 0 && ix < img->width && iy >= 0 && iy < img->height;
    }
    
    //Reduce
    char consist(int ix, int iy) const
    {
        return consist(ix, iy);
    }
    
    char& consist(int ix, int iy)
    {
        assert(in_bounds(ix, iy));
        return consist_data[ix + iy * img->width];
    }
    
    //Resets the consistency data
    void resetConsist() { std::fill(consist_data, consist_data + img->width * img->height, 0); }

    void writeConsist(const std::string& filename);
    
    config save(const std::string& name, const std::string& dir);
    void load(config& data);

private:
    void init(mat44 K_, mat44 R_, mat44 S_);
};

struct VoxelProjection
{
    View* view;
    double verts[2][8];
    int x_center, y_center;
    int x_min, x_max, y_min, y_max;
    int pad;

    static void clamp(int& v, int min, int max) {
        v = v < min ? min : v > max ? max : v;
    }

    void clamp_all() {        
        clamp(x_min, 0, view->img->width - 1); clamp(x_max, 0, view->img->width - 1);
        clamp(y_min, 0, view->img->height - 1); clamp(y_max, 0, view->img->height - 1);
        clamp(x_center, 0, view->img->width - 1); clamp(y_center, 0, view->img->height - 1);
    }

    int size() const {
        return (x_max - x_min) * (y_max - y_min);
    }
        

    VoxelProjection(View* v, vec3 pt) : view(v) {
        vec3 offset[8] = {
            vec3(0, 0, 0),
            vec3(1, 0, 0),
            vec3(0, 1, 0),
            vec3(0, 0, 1),
            vec3(0, 1, 1),
            vec3(1, 0, 1),
            vec3(1, 1, 0),
            vec3(1, 1, 1)
            };
        for (int i = 0; i < 8; i++) {
            vec3 v = pt; v += offset[i];
            vec3 proj = hgmult(view->cam, v);
            verts[0][i] = proj(0); verts[1][i] = proj(1);
        }
        
        vec3 img_loc = hgmult(view->cam, pt);
        x_center = img_loc(0), y_center = img_loc(1);

        pad = 0;
        x_min = * std::min_element(verts[0], verts[0] + 8) - pad;
        y_min = * std::min_element(verts[1], verts[1] + 8) - pad;
        x_max = * std::max_element(verts[0], verts[0] + 8) + pad;
        y_max = * std::max_element(verts[1], verts[1] + 8) + pad;
        assert(x_min <= x_max);
        assert(y_min <= y_max);
        clamp_all();
    }

    void set_pad(int i) {
        int d = i - pad;
        pad = i;
        x_min -= d;
        y_min -= d;
        x_max += d;
        y_max += d;
        clamp_all();
    }
	
    std::pair<int, int> begin() const {
        return std::pair<int, int>(x_min, y_min);
    }
    std::pair<int, int> end() const {
        return std::pair<int, int>(x_min, y_max + 1);
    }
    void next(std::pair<int, int>& p) const {
        if (p.first == x_max) 
            { p.first = x_min; p.second++; }
        else
            p.first++;
    }
};

//Loads a view set from file (using Stanford format)
std::vector<View*>  loadViews(const char* filename, vec3 lo, vec3 hi, ivec3 box, float focal_length = 1);

//Saves a pile of views to an interchange format
void saveTempViews(const std::string& directory, const std::string& filename, std::vector<View*> views);
std::vector<View*> loadTempViews(const std::string& filename);

//Save cameras to PLY
void saveCameraPLY(const char * filename, std::vector<View*> views);

#endif
