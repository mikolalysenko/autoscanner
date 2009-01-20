//Structure from motion module.  Handles problems related to reconstruction.
#ifndef SFM_H
#define SFM_H

#include <vector>
#include <string>

#include "image.h"
#include "view.h"

//Does structure from motion using bundler
std::vector<View>  bundlerSfM(
    std::vector<Image> images, 
    const std::string& bundler_path);
    
//Parses intermediate data from bundler
std::vector<View> parseBundlerTemps(const std::string& directory);

#endif
