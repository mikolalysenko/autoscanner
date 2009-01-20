//This module implements a copy-on-write wrapper for OpenCV's image
#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <Eigen/Core>

#include "opencv.h"
#include "system.h"

    
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
    
    //Color accessors
    Color& operator()(int x, int y)
    {
        assert( 0 <= x && x < image_ptr->width &&
                0 <= y && y < image_ptr->height);
        return *(reinterpret_cast<Color*>(&image_ptr->imageData[3*(x + y * image_ptr->width)]));
    }
    Color  operator()(int x, int y) const
    {
        assert( 0 <= x && x < image_ptr->width &&
                0 <= y && y < image_ptr->height);
        return *(reinterpret_cast<Color*>(&image_ptr->imageData[3*(x + y * image_ptr->width)]));
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
