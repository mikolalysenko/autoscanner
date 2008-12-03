#ifndef HULL_H
#define HULL_H

#include <vector>

#include "misc.h"
#include "volume.h"
#include "view.h"

//Handy aliases for plane directions
extern const vec3 DN[], DU[], DV[];
extern const int N_DIM[], U_DIM[], V_DIM[];

//Computes the photohull within a volume
extern Volume* findHull(
    std::vector<View*> views, 
    int xr, int yr, int zr);

#endif
