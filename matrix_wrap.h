#ifndef MATRIX_WRAP_H
#define MATRIX_WRAP_H

#include <ostream>

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


    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator+(const matrix<matrix_type, matrix_traits>& wrapped, const T& t) {
        matrix_type mat(wrapped.mat_);
        return matrix<matrix_type, matrix_traits>(mat += unwrap(t));
    }
    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator+(const T& t, const matrix<matrix_type, matrix_traits>& wrapped)
        { return wrapped + t; }


    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator-(const matrix<matrix_type, matrix_traits>& wrapped, const T& t) {
        matrix_type mat(wrapped.mat_);
        return matrix<matrix_type, matrix_traits>(mat -= unwrap(t));
    }
    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator-(const T& t, const matrix<matrix_type, matrix_traits>& wrapped)
        { return wrapped - t; }


    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator*(const matrix<matrix_type, matrix_traits>& wrapped, const T& t) {
        matrix_type mat(wrapped.mat_);
        return matrix<matrix_type, matrix_traits>(mat *= unwrap(t));
    }
    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator*(const T& t, const matrix<matrix_type, matrix_traits>& wrapped)
        { return wrapped * t; }


    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator/(const matrix<matrix_type, matrix_traits>& wrapped, const T& t) {
        matrix_type mat(wrapped.mat_);
        return matrix<matrix_type, matrix_traits>(mat /= unwrap(t));
    }
    template <typename T, typename matrix_type, typename matrix_traits>
    const matrix<matrix_type, matrix_traits> operator/(const T& t, const matrix<matrix_type, matrix_traits>& wrapped)
        { return wrapped / t; }
}

#ifdef USE_BLITZPP
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


#endif
