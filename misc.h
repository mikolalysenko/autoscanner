#ifndef MISC_H
#define MISC_H

//STL
#include <utility>
#include <vector>
#include <string>

//Blitz++
#include <blitz/tinyvec.h>
#include <blitz/tinymat.h>




//OpenCV Headers
#ifdef __APPLE__
#  include <OpenCV/OpenCV.h>


//Min/max are missing from STL utility (wtf mac?)
namespace std
{
    template<typename T> T min(T& a, T& b) { return (a < b) ? a : b; }
    template<typename T> T max(T& a, T& b) { return (a > b) ? a : b; }
}

#else
#  include <opencv/cv.h>
#  include <opencv/highgui.h>
#endif

//Vector types
typedef blitz::TinyVector<int, 3>       ivec3;
typedef blitz::TinyVector<int, 4>       ivec4;
typedef blitz::TinyVector<float, 3>     vec3;
typedef blitz::TinyVector<float, 4>     vec4;
typedef blitz::TinyMatrix<float, 3, 3>  mat33;
typedef blitz::TinyMatrix<float, 3, 4>  mat34;
typedef blitz::TinyMatrix<float, 4, 4>  mat44;

//A ray
struct Ray
{
    vec3 o, d;
    
    Ray() {}
    Ray(const Ray& r) : o(r.o), d(r.d) {}
    Ray(const vec3& o_, const vec3 d_) : o(o_), d(d_) {}
    
    Ray operator=(const Ray& r)
    {
        o = r.o; d = r.d;
        return *this;
    }
    
    vec3 operator()(float t) const
    {
        vec3 r = d;
        r *= t;
        r += o;
        return r;
    }
};


//Matrix arithmetic
extern mat44 adjoint(const mat44& m);
extern float det(const mat44& m);
extern mat44 inverse(const mat44& m);
extern mat44 transpose(const mat44& m);
extern mat44 mmult(const mat44& a, const mat44& b);
extern vec3 hgmult(const mat44& a, const vec3& b);
extern vec4 vmult(const mat44& a, const vec4& b);
extern float len(const vec3& v);

//Retrieves temporary directory
extern char * getTempDirectory();

extern void savePly(const std::string& filename, const std::vector<vec3>& points);


#endif
