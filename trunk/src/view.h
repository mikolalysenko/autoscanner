#ifndef VIEW_H
#define VIEW_H

//STL includes
#include <cassert>
#include <vector>
#include <string>

//Eigen
#include <Eigen/Core>

//Project files
#include "config.h"
#include "image.h"

//A camera view, stores a reference to an image and a camera matrix
struct View
{
    //Default constructors
    View();
    View(const View& other);
    
    //Construction from parameters
    View(const Image image, Eigen::Matrix4f cam);
    
    //TODO: Implement serialization for debugging
    
    //Accessors
    Eigen::Vector3f center() const;
    Eigen::Matrix4f camera() const      { return cam; }
    Image image() const                 { return img; }
    
private:
    //Image data
    Image img;
    
    //Camera parameters
    Eigen::Matrix4f cam;
};


#endif
