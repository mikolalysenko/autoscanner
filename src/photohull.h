#ifndef HULL_H
#define HULL_H

#include <vector>

#include "volume.h"
#include "view.h"

#include <Eigen/Core>

//Computes the photohull within a volume
extern Volume* findHull(
    std::vector<View*> views, 
    int xr, int yr, int zr,
    Eigen::Vector3f low, 
    Eigen::Vector3f high);


#endif
