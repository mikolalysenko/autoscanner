#ifndef VIDEO_LOADER_H
#define VIDEO_LOADER_H

#include <vector>
#include <string>
#include "view.h"

#include <Eigen/Core>

//Path to the bundler script
extern const char* bundler_path;

//Loads a video sequence, calls bundler
std::vector<View*> loadVideo(
    const char * video_file, 
    Vector3i grid_dim, 
    Vector3f& low, 
    Vector3f& high);

//Loads up temporary data
std::vector<View*> loadTempBundleData(
    const char* directory, 
    Vector3i grid_dim, 
    Vector3f& box_min, 
    Vector3f& box_max);


#endif
