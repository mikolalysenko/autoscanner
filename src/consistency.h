#ifndef CONSISTENCY_H
#define CONSISTENCY_H

#include "volume.h"

#include <vector>

#include <Eigen/Core>

//Check if a set of neighborhoods is photoconsistent
extern bool checkNeighborhood(vector<VoxelProjection>& patches) ;

//Checks photoconsistency of a voxel in the volume
extern bool checkConsistency(
    std::vector<View*>& views,
    Volume* volume,
    Eigen::Vector3i point,
    int d) ;



#endif
