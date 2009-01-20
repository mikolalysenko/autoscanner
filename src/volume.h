#ifndef VOLUME_H
#define VOLUME_H

#include <algorithm>
#include <cassert>
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include <Eigen/Core>
#include <Eigen/Array>

#include "system.h"

//Voxel data structure
struct Volume
{
    //Default constructors
    Volume() {}
    Volume(const Volume& other) :
        xRes(other.xRes), yRes(other.yRes), zRes(other.zRes),
        colors(other.colors),
        mat(other.mat) {}
            
    //Constructs a matrix from preallocated memory
    Volume(
        const Eigen::Vector3i& dimensions,
        std::vector<Color> data,
        const Eigen::Matrix4d& transform) :
            xRes(dimensions.x()), yRes(dimensions.y()), zRes(dimensions.z()),
            colors(data),
            mat(new Eigen::Matrix4d(transform)) {}
    
    //Default volume constructor
    Volume(Eigen::Vector3i dimensions,
            Eigen::Vector3d low_bound, 
            Eigen::Vector3d high_bound) :
        xRes(dimensions.x()), 
        yRes(dimensions.y()), 
        zRes(dimensions.z()),
        colors(dimensions.x() * dimensions.y() * dimensions.z())
    {
        Eigen::Matrix4d m = Eigen::Matrix4d::Zero();
        m.block(0,3,0,3) += (Eigen::Vector3d(dimensions).cwise() / 
            (high_bound - low_bound)).asDiagonal();
        m.block(0,3,3,1) += low_bound;
        m(3,3) = 1;
        mat = boost::shared_ptr<Eigen::Matrix4d>(new Eigen::Matrix4d(m));
        
        fill(Color(255, 255, 255));
    }

    //Assignment operator
    Volume operator=(const Volume& other)
    {
        xRes = other.xRes;
        yRes = other.yRes;
        zRes = other.zRes;
        colors = other.colors;
        mat = other.mat;
        return *this;
    }
    
    //Fills the volume with some arbitrary color
    void fill(const Color& color)
    {
        std::fill(colors.begin(), colors.end(), color);
    }    
    
    //Saving for debugging
    void save(const std::string filename) const;
    void savePLY(const std::string filename) const;
    
    //Retrieves size
    Eigen::Vector3i size() const { return Eigen::Vector3i(xRes, yRes, zRes); }
    
    //Color array access
    Color& operator()(const Eigen::Vector3i& v)
    {
        assert((size_t)v.x() < xRes && (size_t)v.y() < yRes && (size_t)v.z() < zRes);
        return colors[v.x() + xRes * (v.y() + yRes * v.z())];
    }
    Color operator()(const Eigen::Vector3i& v) const
    {
        assert((size_t)v.x() < xRes && (size_t)v.y() < yRes && (size_t)v.z() < zRes);
        return colors[v.x() + xRes * (v.y() + yRes * v.z())];
    }
    
    //Point membership classification
    bool interior(const Eigen::Vector3i& v) const
    {
        return (*this)(v) != Color(0,0,0);
    }
    bool exterior(const Eigen::Vector3i& v) const
    {
        return (*this)(v) == Color(0,0,0);
    }
    bool surface(const Eigen::Vector3i& v) const
    {
        return 
            interior(v) && (
                exterior(v+Eigen::Vector3i( 1, 0, 0)) ||
                exterior(v+Eigen::Vector3i(-1, 0, 0)) ||
                exterior(v+Eigen::Vector3i( 0, 1, 0)) ||
                exterior(v+Eigen::Vector3i( 0,-1, 0)) ||
                exterior(v+Eigen::Vector3i( 0, 0, 1)) ||
                exterior(v+Eigen::Vector3i( 0, 0,-1)) );
    }
    
    //Matrix coordinates
    Eigen::Matrix4d  xform() const { return *mat; }
    Eigen::Matrix4d& xform()
    {
        if(mat.unique())
            return *mat; 
        return *(mat = boost::shared_ptr<Eigen::Matrix4d>(new Eigen::Matrix4d(*mat)));
    }

private:
    
    //Voxel grid dimensions
    size_t xRes, yRes, zRes;
    std::vector<Color> colors;

    //World -> volume coordinate transform
    boost::shared_ptr<Eigen::Matrix4d> mat;
};

#endif
