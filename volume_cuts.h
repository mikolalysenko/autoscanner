#ifndef VOLUME_CUTS_H
#define VOLUME_CUTS_H

#include <vector>

#include "misc.h"
#include "view.h"
#include "volume.h"

//Finds volumetric graph cut based object
Volume* volumetricGraphCuts(
    std::vector<View*> views,
    vec3 box_min, vec3 box_max,
    ivec3 grid_res,
    vec3 interior_point,
    Volume * photo_hull);

#endif
