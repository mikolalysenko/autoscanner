#ifndef VIEW_H
#define VIEW_H

//STL includes
#include <cassert>
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

//Eigen
#include <Eigen/Core>
#include <Eigen/Geometry>

//Project files
#include "image.h"
#include "system.h"

//A camera view, stores a reference to an image and a camera matrix
struct View
{
    //Default constructors
    View();
    View(const View& other) :
        img(other.img), R(other.R), K(other.K) {}
    
    //Construction from parameters
    View(const Image img_, Eigen::Matrix4d R_, Eigen::Matrix4d K_) :
        img(img_), 
        R(new Eigen::Matrix4d(R_)), 
        K(new Eigen::Matrix4d(K_))  {}
    
    //Assignment operator
    View operator=(const View& other)
    {
        img = other.img;
        R = other.R;
        K = other.K;
        return *this;
    }
    
    //Matrix accessors
    Eigen::Vector3d center() const          { return R->block(0,3,3,1); }
    Eigen::Matrix3d rotation() const        { return R->block(0,3,0,3); }
    Eigen::Transform3d intrinsic() const    { return Eigen::Transform3d(*K); }
    Eigen::Transform3d camera() const       { return Eigen::Transform3d((*K) * (*R)); }
    Eigen::Transform3d world() const        { return Eigen::Transform3d(*R); }
    
    //Image accessor
    Image image() const                 { return img; }
    
private:
    //Image data
    Image img;
    
    //World matrix (stores position/rotation)
    boost::shared_ptr<Eigen::Matrix4d> R;
    
    //Intrinsic matrix (stores focal length, local image trnasform)
    boost::shared_ptr<Eigen::Matrix4d> K;
};


#endif
