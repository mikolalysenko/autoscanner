#ifndef CONSISTENCY_H
#define CONSISTENCY_H


#include "misc.h"
#include "volume.h"

#include <vector>

//Check if a set of neighborhoods is photoconsistent
extern bool checkNeighborhood(vector<VoxelProjection>& patches) ;

//Checks photoconsistency of a voxel in the volume
extern bool checkConsistency(
    std::vector<View*>& views,
    Volume* volume,
    ivec3 point,
    int d) ;



#endif
