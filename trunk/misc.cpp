#include <iostream>
#include <cmath>
#include <string>
#include <fstream>

#include "misc.h"

using namespace std;

char * getTempDirectory()
{
    if(getenv("TMPDIR"))
        return getenv("TMPDIR");
    if(getenv("TEMP"))
        return getenv("TEMP");
    if(getenv("TMP"))
        return getenv("TMP");
    return "/tmp/";
}

float len(const vec3& v)
{
    return sqrtf(v(0) * v(0) + v(1) * v(1) + v(2) * v(2));
}


mat44 adjoint(const mat44& m)
{
    mat44 r;
    
    r(0,0)=	-m(1,3)*m(2,2)*m(3,1) + m(1,2)*m(2,3)*m(3,1) + m(1,3)*m(2,1)*m(3,2) - m(1,1)*m(2,3)*m(3,2) - m(1,2)*m(2,1)*m(3,3) + m(1,1)*m(2,2)*m(3,3); 
    r(0,1)=	 m(0,3)*m(2,2)*m(3,1) - m(0,2)*m(2,3)*m(3,1) - m(0,3)*m(2,1)*m(3,2) + m(0,1)*m(2,3)*m(3,2) + m(0,2)*m(2,1)*m(3,3) - m(0,1)*m(2,2)*m(3,3); 
    r(0,2)=	-m(0,3)*m(1,2)*m(3,1) + m(0,2)*m(1,3)*m(3,1) + m(0,3)*m(1,1)*m(3,2) - m(0,1)*m(1,3)*m(3,2) - m(0,2)*m(1,1)*m(3,3) + m(0,1)*m(1,2)*m(3,3);
    r(0,3)=	 m(0,3)*m(1,2)*m(2,1) - m(0,2)*m(1,3)*m(2,1) - m(0,3)*m(1,1)*m(2,2) + m(0,1)*m(1,3)*m(2,2) + m(0,2)*m(1,1)*m(2,3) - m(0,1)*m(1,2)*m(2,3);
    r(1,0)=	 m(1,3)*m(2,2)*m(3,0) - m(1,2)*m(2,3)*m(3,0) - m(1,3)*m(2,0)*m(3,2) + m(1,0)*m(2,3)*m(3,2) + m(1,2)*m(2,0)*m(3,3) - m(1,0)*m(2,2)*m(3,3);
    r(1,1)=	-m(0,3)*m(2,2)*m(3,0) + m(0,2)*m(2,3)*m(3,0) + m(0,3)*m(2,0)*m(3,2) - m(0,0)*m(2,3)*m(3,2) - m(0,2)*m(2,0)*m(3,3) + m(0,0)*m(2,2)*m(3,3);
    r(1,2)=	 m(0,3)*m(1,2)*m(3,0) - m(0,2)*m(1,3)*m(3,0) - m(0,3)*m(1,0)*m(3,2) + m(0,0)*m(1,3)*m(3,2) + m(0,2)*m(1,0)*m(3,3) - m(0,0)*m(1,2)*m(3,3);
    r(1,3)=	-m(0,3)*m(1,2)*m(2,0) + m(0,2)*m(1,3)*m(2,0) + m(0,3)*m(1,0)*m(2,2) - m(0,0)*m(1,3)*m(2,2) - m(0,2)*m(1,0)*m(2,3) + m(0,0)*m(1,2)*m(2,3);
    r(2,0)=	-m(1,3)*m(2,1)*m(3,0) + m(1,1)*m(2,3)*m(3,0) + m(1,3)*m(2,0)*m(3,1) - m(1,0)*m(2,3)*m(3,1) - m(1,1)*m(2,0)*m(3,3) + m(1,0)*m(2,1)*m(3,3); 
    r(2,1)=	 m(0,3)*m(2,1)*m(3,0) - m(0,1)*m(2,3)*m(3,0) - m(0,3)*m(2,0)*m(3,1) + m(0,0)*m(2,3)*m(3,1) + m(0,1)*m(2,0)*m(3,3) - m(0,0)*m(2,1)*m(3,3);
    r(2,2)=	-m(0,3)*m(1,1)*m(3,0) + m(0,1)*m(1,3)*m(3,0) + m(0,3)*m(1,0)*m(3,1) - m(0,0)*m(1,3)*m(3,1) - m(0,1)*m(1,0)*m(3,3) + m(0,0)*m(1,1)*m(3,3);
    r(2,3)=	 m(0,3)*m(1,1)*m(2,0) - m(0,1)*m(1,3)*m(2,0) - m(0,3)*m(1,0)*m(2,1) + m(0,0)*m(1,3)*m(2,1) + m(0,1)*m(1,0)*m(2,3) - m(0,0)*m(1,1)*m(2,3);
    r(3,0)=	 m(1,2)*m(2,1)*m(3,0) - m(1,1)*m(2,2)*m(3,0) - m(1,2)*m(2,0)*m(3,1) + m(1,0)*m(2,2)*m(3,1) + m(1,1)*m(2,0)*m(3,2) - m(1,0)*m(2,1)*m(3,2);
    r(3,1)=	-m(0,2)*m(2,1)*m(3,0) + m(0,1)*m(2,2)*m(3,0) + m(0,2)*m(2,0)*m(3,1) - m(0,0)*m(2,2)*m(3,1) - m(0,1)*m(2,0)*m(3,2) + m(0,0)*m(2,1)*m(3,2);
    r(3,2)=	 m(0,2)*m(1,1)*m(3,0) - m(0,1)*m(1,2)*m(3,0) - m(0,2)*m(1,0)*m(3,1) + m(0,0)*m(1,2)*m(3,1) + m(0,1)*m(1,0)*m(3,2) - m(0,0)*m(1,1)*m(3,2);
    r(3,3)=	-m(0,2)*m(1,1)*m(2,0) + m(0,1)*m(1,2)*m(2,0) + m(0,2)*m(1,0)*m(2,1) - m(0,0)*m(1,2)*m(2,1) - m(0,1)*m(1,0)*m(2,2) + m(0,0)*m(1,1)*m(2,2);
    
    return r;
}

float det(const mat44& m)
{
    return      ((m(3,3)*m(2,2) - m(3,2)*m(2,3))*m(1,1) +
				((-m(3,3)*m(2,1) + m(3,1)*m(2,3))*m(1,2) + 
				(  m(3,2)*m(2,1) - m(3,1)*m(2,2))*m(1,3)))*m(0,0) + 
               (((-m(3,3)*m(2,2) + m(3,2)*m(2,3))*m(1,0) + 
				(( m(3,3)*m(2,0) - m(3,0)*m(2,3))*m(1,2) + 
				( -m(3,2)*m(2,0) + m(3,0)*m(2,2))*m(1,3)))*m(0,1) + 
				(((m(3,3)*m(2,1) - m(3,1)*m(2,3))*m(1,0) + 
				((-m(3,3)*m(2,0) + m(3,0)*m(2,3))*m(1,1) + 
				(  m(3,1)*m(2,0) - m(3,0)*m(2,1))*m(1,3)))*m(0,2) + 
				((-m(3,2)*m(2,1) + m(3,1)*m(2,2))*m(1,0) + 
				(( m(3,2)*m(2,0) - m(3,0)*m(2,2))*m(1,1) + 
				( -m(3,1)*m(2,0) + m(3,0)*m(2,1))*m(1,2)))*m(0,3)));
}


mat44 inverse(const mat44& m)
{
    mat44 r = adjoint(m);
    float d = det(m);
    
    assert(abs(d) > 1e-10);
    
    for(int i=0; i<4; i++)
    for(int j=0; j<4; j++)
        r(i,j) /= d;
    
    return r;
}

mat44 transpose(const mat44& m)
{
    mat44 r;
    
    for(int i=0; i<4; i++)
    for(int j=0; j<4; j++)
        r(i,j) = m(j,i);
    
    return r;
}

mat44 mmult(const mat44& a, const mat44& b)
{
    mat44 r;
    
    for(int i=0; i<4; i++)
    for(int j=0; j<4; j++)
    {
        r(i,j) = 0.0f;
        for(int k=0; k<4; k++)
            r(i,j) += a(i,k) * b(k,j);
    }
    
    return r;
}


vec4 vmult(const mat44& a, const vec4& b)
{
    vec4 r;
    
    for(int i=0; i<4; i++)
    {
        r(i) = 0.0f;
        for(int j=0; j<4; j++)
            r(i) += a(i,j) * b(j);
    }
    
    return r;
}

vec3 hgmult(const mat44& a, const vec3& b)
{
    vec4 r = vmult(a, vec4(b(0), b(1), b(2), 1.0f));
    
    return vec3(r(0) / r(3), r(1) / r(3), r(2) / r(3));
}

void savePly(const string& filename, const vector<vec3>& points) {
    typedef const vector<vec3> pointVector;
    
    ofstream fout(filename.c_str(), ios_base::out | ios_base::trunc);

    fout << "ply" << endl;
    fout << "format ascii 1.0" << endl;
    fout << "comment output from autoscanner" << endl;
    fout << "element vertex " << points.size() << endl;
    fout << "property float x" << endl;
    fout << "property float y" << endl;
    fout << "property float z" << endl;
    fout << "end_header" << endl;

    for (pointVector::const_iterator iter = points.begin(); iter != points.end(); iter++)
        fout << (*iter)(0) << " " << (*iter)(1) << " " << (*iter)(2) << endl;

    fout.close();

}

