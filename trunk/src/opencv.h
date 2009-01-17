#ifndef OPENCV_H
#define OPENCV_H

//This file imports the necessary OpenCV files
//  Stupid workaround for MacOS X bullshit
#ifdef __APPLE__
#  include <OpenCV/OpenCV.h>
#else
#  include <opencv/cv.h>
#  include <opencv/highgui.h>
#endif

#endif
