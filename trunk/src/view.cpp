#include "image.h"
#include "view.h"

#include <Eigen/Core>

USING_PART_OF_NAME_NAMESPACE_EIGEN
using namespace std;
using namespace blitz;

//Retrieves the center of the view
Vector3f View::center() const
{
    Vector4f z = cam.inverse() * Vector4f(0, 0, 0, 1);
    return Vector3f(z.x(), z.y(), z.z()) / z.w();
}

