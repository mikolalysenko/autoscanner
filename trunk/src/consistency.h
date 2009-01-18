#ifndef CONSISTENCY_H
#define CONSISTENCY_H

#include "volume.h"

#include <vector>

#include <Eigen/Core>

//Moved here from view.h
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
        Eigen::Vector3f offset[8] = {
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
            Eigen::Vector3f v = pt; v += offset[i];
            Eigen::Vector3f proj = hgmult(view->cam, v);
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


//Check if a set of neighborhoods is photoconsistent
extern bool checkNeighborhood(std::vector<VoxelProjection>& patches) ;

//Checks photoconsistency of a voxel in the volume
extern bool checkConsistency(
    std::vector<View*>& views,
    Volume* volume,
    Eigen::Vector3i point,
    int d) ;



#endif
