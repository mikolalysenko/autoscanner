#ifndef HULL_H
#define HULL_H

#include <vector>

#include "volume.h"
#include "view.h"

#include <Eigen/Core>

//Computes a photohull from a set of views
extern Volume stereoPhotoHull(
    std::vector<View> views, 
    Eigen::Vector3i dim,
    Eigen::Vector3d low, 
    Eigen::Vector3d high);


//TODO: Add other stereo methods

#endif
