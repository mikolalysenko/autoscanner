#ifndef KUTULAKIS_H
#define KUTULAKIS_H

#include "misc.h"
#include "config.h"

#include <string>

//loads one camera
extern cfg::config openKutulakisCamera(const std::string& dir, const std::string& filename, vec3 lo, vec3 high, ivec3 box);

//loads many cameras
extern cfg::config loadKutulakisCameras(const std::string& dir, const std::string& filename, vec3 lo, vec3 high, ivec3 box);

#endif

