#ifndef HULL_H
#define HULL_H

#include <vector>

#include "misc.h"
#include "volume.h"
#include "view.h"

//Computes the photohull within a volume
extern Volume* findHull(
    std::vector<View*> views, 
    int xr, int yr, int zr);

#endif
