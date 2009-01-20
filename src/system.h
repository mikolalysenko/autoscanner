//This module includes system specific functions which require
//non-portable library calls to get working (ex. finding /tmp/)
#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include <string>

#include <Eigen/Core>

//Alignment macro
#define ALIGN16         __attribute__ ((aligned (16)))

//8-bit sizes
typedef char                    byte;
typedef unsigned char           ubyte;

//Clamp/saturate color components
template<typename T> T clamp(T a, T l, T h)     { return min(max(a, l), h); }
template<typename T> T saturate(T a)            { return clamp(a, (T)0, (T)1); }

//Retrieves temporary directory
extern std::string getTempDirectory();

//Color data type with interface to Eigen
// Somewhat tedious, but necessary due to the fact that Eigen's internal memory layout is not
// compatible with the Color format used by OpenCV.
struct Color
{
    
    //Default ctors
    Color() : b(0), g(0), r(0) {}
    Color(const Color& p) : b(p.b), g(p.g), r(p.r) {}
    Color(ubyte r_, ubyte g_, ubyte b_) : b(b_), g(g_), r(r_) {}
    
    //Vector -> Color cast operators
    Color(const Eigen::Vector3i& v) : b(v.z()), g(v.y()), r(v.x()) {}
    Color(const Eigen::Vector3f& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
    }
    Color(const Eigen::Vector3d& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
    }
    
    //Assignment overloads
    Color operator=(const Color& v)
    {
        r = v.r;
        g = v.g;
        b = v.b;
        return *this;
    }
    Color operator=(const Eigen::Vector3i& v)
    {
        r = v.x();
        g = v.y();
        b = v.z();
        return *this;
    }
    Color operator=(const Eigen::Vector3f& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
        return *this;
    }
    Color operator=(const Eigen::Vector3d& v)
    {
        r = ubyte(saturate(v.x()) * 255.0f);
        g = ubyte(saturate(v.y()) * 255.0f);
        b = ubyte(saturate(v.z()) * 255.0f);
        return *this;
    }
    
    //Color -> Vector cast operators
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
    
    //Equality / comparison tests
    bool operator==(const Color& other) const
    {
        return
            b == other.b && 
            g == other.g && 
            r == other.r;
    }
    bool operator!=(const Color& other) const
    {
        return
            b != other.b ||
            g != other.g ||
            r != other.r;
    }
    bool operator<(const Color& other) const
    {
        return
            r < other.r ||
            (r == other.r &&
                (g < other.g ||
                (g == other.g &&
                    b < other.b)));
    }
    
    //Converts Color to a luminance value
    double luminance() const
    {
        return (0.3 * (double)r + 0.59 * (double)g + 0.11 * (double)b) / 255.0;
    }

    //Color channels
    ubyte b, g, r;
};

//Print out contents of a Color
extern std::ostream& operator<<(std::ostream& os, const Color& p);


#endif
