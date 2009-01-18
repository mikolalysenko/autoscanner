//This module implements a copy-on-write wrapper for OpenCV's image
#ifndef IMAGE_H
#define IMAGE_H

#include <boost/shared_ptr.hpp>

#include <Eigen/Core>

#include "opencv.h"
#include "system.h"

    

//Pixel data type with interface to Eigen
// Somewhat tedious, but necessary due to the fact that Eigen's internal memory layout is not
// compatible with the pixel format used by OpenCV.
struct Pixel
{
    
    //Default ctors
    Pixel() : b(0), g(0), r(0) {}
    Pixel(const Pixel& p) : b(p.b), g(p.g), r(p.r) {}
    Pixel(ubyte r_, ubyte g_, ubyte b_) : b(b_), g(g_), r(r_) {}
    
    //Vector -> Pixel cast operators
    Pixel(const Eigen::Vector3i& v) : b(v.z()), g(v.y()), r(v.x()) {}
    Pixel(const Eigen::Vector3f& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
    }
    Pixel(const Eigen::Vector3d& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
    }
    
    //Assignment overloads
    Pixel operator=(const Pixel& v)
    {
        r = v.r;
        g = v.g;
        b = v.b;
        return *this;
    }
    Pixel operator=(const Eigen::Vector3i& v)
    {
        r = v.x();
        g = v.y();
        b = v.z();
        return *this;
    }
    Pixel operator=(const Eigen::Vector3f& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
        return *this;
    }
    Pixel operator=(const Eigen::Vector3d& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
        return *this;
    }
    
    //Pixel -> Vector cast operators
    operator Eigen::Vector3i () const
    {
        return Eigen::Vector3i(r, g, b);
    }
    operator Eigen::Vector3f () const
    {
        return Eigen::Vector3f(
            (float)r / 255.0f,
            (float)g / 255.0f,
            (float)b / 255.0f);
    }
    operator Eigen::Vector3d () const
    {
        return Eigen::Vector3d(
            (double)r / 255.0,
            (double)g / 255.0,
            (double)b / 255.0);
    }
    
    //Converts pixel to a luminance value
    double luminance() const
    {
        return (0.3 * (double)r + 0.59 * (double)g + 0.11 * (double)b) / 255.0;
    }

    //Color channels
    ubyte b, g, r;
};

//Print out contents of a pixel
extern std::ostream& operator<<(std::ostream& os, const Pixel& p);

//Wrapper for OpenCV's IPLImage data structure, uses copy-on-write semantics
//Not thread safe in the least, but who cares?
struct Image
{
    //NULL constructor
    Image() {}
    
    //Copy ctor
    Image(const Image& img) : image_ptr(img.image_ptr) {}
    
    //Places img* under control of this object.
    //WARNING: Will delete img* if no longer referenced
    Image(IplImage * img);
    
    //Create image from file
    Image(const std::string& filename);
    
    //Creates new image with specified dimensions
    Image(int w, int h);
    
    //Duplicate image
    Image operator=(const Image& rhs) { image_ptr = rhs.image_ptr; return *this; }
    
    //Create a duplicate image
    Image dup() const;
    
    //Dimension accessors
    int height()    const { return image_ptr->height; }
    int width()     const { return image_ptr->width; }
    int widthStep() const { return image_ptr->widthStep; }
    
    //Cast to IplImage*
    operator IplImage* ()             { check_copy(); return image_ptr.get(); }
    operator const IplImage* () const { return image_ptr.get(); }
    
    //Cast to ubyte* (useful for unsafe but fast memory access)
    operator ubyte* ()             { check_copy(); return (ubyte*)image_ptr->imageData; }
    operator const ubyte* () const { return (const ubyte*)image_ptr->imageData; }
    
    //Pixel accessors
    Pixel& operator()(int x, int y)
    {
        assert( 0 <= x && x < image_ptr->width &&
                0 <= y && y < image_ptr->height);
        return *(reinterpret_cast<Pixel*>(&image_ptr->imageData[3*(x + y * image_ptr->width)]));
    }
    Pixel  operator()(int x, int y) const
    {
        assert( 0 <= x && x < image_ptr->width &&
                0 <= y && y < image_ptr->height);
        return *(reinterpret_cast<Pixel*>(&image_ptr->imageData[3*(x + y * image_ptr->width)]));
    }
    
    //Resize image, returns result
    Image resize(int w, int h) const;
    
    //Disk serialization
    void save(const std::string& filename) const;
    
private:
    //Pointer to actual IplImage
    boost::shared_ptr<IplImage> image_ptr;

    //Check for a copy after a write
    void check_copy();

    //Sets the image shared_ptr to img, updates other state variables
    void set_image(IplImage * img);
};


//Reads a movie file from disk and chops it into a set of pictures
extern std::vector<Image> loadMovie(const std::string& filename, int frameskip = 0);

#endif
