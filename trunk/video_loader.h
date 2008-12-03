#ifndef VIDEO_LOADER_H
#define VIDEO_LOADER_H

#include <vector>
#include <string>
#include "view.h"

//Path to the bundler script
extern const char* bundler_path;

//Loads a video sequence, calls bundler
std::vector<View*> loadVideo(const char * video_file);

#endif
