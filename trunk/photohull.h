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
    int xr, int yr, int zr,
    vec3 low, vec3 high);

extern const vec3 DN[6], DU[6], DV[6];

struct Cone
{
    vec4 cone[4];

    Cone(vec3 dn, vec3 du, vec3 dv, vec3 pt) {
        cone[0] = cone[1] = cone[2] = cone[3] = vec4(dn(0), dn(1), dn(2), 0);
        
        cone[0] += vec4(du(0), du(1), du(2), 0);
        cone[1] -= vec4(du(0), du(1), du(2), 0);
        cone[2] += vec4(dv(0), dv(1), dv(2), 0);
        cone[3] -= vec4(dv(0), dv(1), dv(2), 0);
        
        for(int i=0; i<4; i++)
        {
            cone[i](3) = -(cone[i](0) * pt(0) + 
                           cone[i](1) * pt(1) +
                           cone[i](2) * pt(2));
        }
    }

    bool contains(vec3 pt) {
        for(int j=0; j<4; j++)
        {
            double d = 
                cone[j](0) * pt(0) +
                cone[j](1) * pt(1) +
                cone[j](2) * pt(2) +
                cone[j][3];
            
            if(d > 0.0f)
                return false;
        }
        return true;
    }

};

#endif
