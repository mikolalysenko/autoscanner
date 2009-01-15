#ifndef MATRIX_WRAP_H
#define MATRIX_WRAP_H

#include <ostream>


// matrix_wrap.h shouldn't be included explicitly, only through misc.h since misc.h possibly defines
// constants that we use. Let's ensure we get those.
#include "misc.h"

namespace matrix {

    template <typename matrix_type, typename matrix_traits> struct matrix;

    template <typename T>
    struct unwrapper {
        typedef T type;
        static type& unwrap(T& t) { return t; }
    };

    template <typename T, typename M_T>
    struct unwrapper<matrix<T, M_T> > {
        typedef T type;
        static type& unwrap(matrix<T, M_T>& wrapped_mat) { return wrapped_mat.mat_; }
    };

    
    template <typename T, typename M_T>
    struct unwrapper<const matrix<T, M_T> > {
        typedef const T type;
        static type& unwrap(const matrix<T, M_T>& wrapped_mat) { return wrapped_mat.mat_; }
    };


    template <typename T>
    typename unwrapper<T>::type& unwrap(T& t) { return unwrapper<T>::unwrap(t); }

    template <typename number_type>
    struct matrix_traits {
        typedef number_type num_type;
    };

    template <typename type, typename matrix_traits>
    struct matrix {
        typedef typename matrix_traits::num_type num_type;
        typedef type matrix_type;

        matrix_type mat_;

    private:  
        operator matrix_type() { return mat_; }

    public:
        matrix() : mat_() { }

        template <typename o_matrix_type, typename o_matrix_traits>
        matrix(const matrix<o_matrix_type, o_matrix_traits>& o) : mat_(o.mat_) { }

        template <typename U>
        matrix(const U& u) : mat_(unwrap(u)) { }

        template <typename U>
        matrix& operator=(const U& u) { mat_ = unwrap(u); return *this; }

        template <typename U, typename R>
        matrix(const U& u, const R& r) : mat_(unwrap(u), unwrap(r)) { }

        template <typename U, typename R, typename S>
        matrix(const U& u, const R& r, const S& s) : mat_(unwrap(u), unwrap(r), unwrap(s)) { }

        template <typename U, typename R, typename S, typename Q>
        matrix(const U& u, const R& r, const S& s, const Q& q) : mat_(unwrap(u), unwrap(r), unwrap(s), unwrap(q)) { }
        
        template <typename U>
        matrix& operator+=(const U& u) { mat_ += unwrap(u); return *this; }       
        
        template <typename U>
        matrix& operator-=(const U& u) { mat_ -= unwrap(u); return *this; }

        template <typename U>
        matrix& operator*=(const U& u) { mat_ *= unwrap(u); return *this; }

        template <typename U>
        matrix& operator/=(const U& u) { mat_ /= unwrap(u); return *this; }

        num_type& operator[](int i) { return mat_[i]; }
        num_type& operator()(int i) { return mat_[i]; }
        num_type operator[](int i) const { return mat_[i]; }
        num_type operator()(int i) const { return mat_[i]; }

        num_type& operator()(int i, int j) { return mat_(i,j); }
        num_type operator()(int i, int j) const { return mat_(i,j); }
    };

    template <typename matrix_type, typename matrix_traits>
    std::ostream& operator<<(std::ostream& o, const matrix<matrix_type, matrix_traits>& m) { return o << unwrap(m); }
}

#ifdef MW_USE_BLITZPP
    #define MW_NEEDS_BINARY_OPERATORS
    //Blitz++
    #include <blitz/tinyvec.h>
    #include <blitz/tinymat.h>
    //Vector types
    typedef matrix::matrix<blitz::TinyVector<int, 3>, matrix::matrix_traits<int> > ivec3;
    typedef matrix::matrix<blitz::TinyVector<int, 4>, matrix::matrix_traits<int> > ivec4;
    typedef matrix::matrix<blitz::TinyVector<float, 3>, matrix::matrix_traits<float> > vec3;
    typedef matrix::matrix<blitz::TinyVector<float, 4>, matrix::matrix_traits<float> > vec4;
    typedef matrix::matrix<blitz::TinyMatrix<float, 3, 4>, matrix::matrix_traits<float> > mat33;
    typedef matrix::matrix<blitz::TinyMatrix<float, 3, 4>, matrix::matrix_traits<float> > mat34;
    typedef matrix::matrix<blitz::TinyMatrix<float, 4, 4>, matrix::matrix_traits<float> > mat44;
#else

#endif


#ifdef MW_NEEDS_BINARY_OPERATORS
    namespace matrix {

        template <typename T, typename U>
        struct binary_operator;

        template <typename T, typename matrix_type, typename matrix_traits>
        struct binary_operator<T, matrix<matrix_type, matrix_traits> > {
            typedef const matrix<matrix_type, matrix_traits> return_type;
            typedef const T& left;
            typedef const matrix<matrix_type, matrix_traits>& right;
            static return_type add(left l, right r) { matrix_type m(r.mat_); return (m += unwrap(l)); }
            static return_type subtract(left l, right r) { matrix_type m(r.mat_); return (m -= unwrap(l)); }
            static return_type multiply(left l, right r) { matrix_type m(r.mat_); return (m *= unwrap(l)); }
            static return_type divide(left l, right r) { matrix_type m(r.mat_); return (m /= unwrap(l)); }
        };

        template <typename T, typename matrix_type, typename matrix_traits>
        struct binary_operator<matrix<matrix_type, matrix_traits>, T> {
            typedef const matrix<matrix_type, matrix_traits> return_type;
            typedef const T& right;
            typedef const matrix<matrix_type, matrix_traits>& left;
            static return_type add(left l, right r) { matrix_type m(l.mat_); return (m += unwrap(r)); }
            static return_type subtract(left l, right r) { matrix_type m(l.mat_); return (m -= unwrap(r)); }
            static return_type multiply(left l, right r) { matrix_type m(l.mat_); return (m *= unwrap(r)); }
            static return_type divide(left l, right r) { matrix_type m(l.mat_); return (m /= unwrap(r)); }
        };

        template <typename matrix_type, typename matrix_traits, typename o_matrix_type, typename o_matrix_traits>
        struct binary_operator<matrix<matrix_type, matrix_traits>, matrix<o_matrix_type, o_matrix_traits> > {
            typedef const matrix<matrix_type, matrix_traits> return_type;
            typedef const matrix<o_matrix_type, o_matrix_traits>& right;
            typedef const matrix<matrix_type, matrix_traits>& left;
            static return_type add(left l, right r) { matrix_type m(l.mat_); return (m += unwrap(r)); }
            static return_type subtract(left l, right r) { matrix_type m(l.mat_); return (m -= unwrap(r)); }
            static return_type multiply(left l, right r) { matrix_type m(l.mat_); return (m *= unwrap(r)); }
            static return_type divide(left l, right r) { matrix_type m(l.mat_); return (m /= unwrap(r)); }
        };
            

        template <typename T, typename U>
        typename binary_operator<T, U>::return_type operator+(const T& t, const U& u) { return binary_operator<T, U>::add(t, u); }

        template <typename T, typename U>
        typename binary_operator<T, U>::return_type operator-(const T& t, const U& u) { return binary_operator<T, U>::subtract(t, u); }

        template <typename T, typename U>
        typename binary_operator<T, U>::return_type operator*(const T& t, const U& u) { return binary_operator<T, U>::multiply(t, u); }

        template <typename T, typename U>
        typename binary_operator<T, U>::return_type operator/(const T& t, const U& u) { return binary_operator<T, U>::divide(t, u); }
    }
#endif


#endif
