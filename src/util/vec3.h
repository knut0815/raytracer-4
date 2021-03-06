/* MIT License
Copyright (c) 2018 Biro Eniko
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "util/common.h"

class Vec3
{

    float e[3];
    
    public:
        CUDA_HOSTDEV Vec3() {}
        CUDA_HOSTDEV Vec3(float x, float y, float z)
        {
            e[0] = x;
            e[1] = y;
            e[2] = z;
        }
        CUDA_HOSTDEV float x() const {return e[0];}
        CUDA_HOSTDEV float y() const {return e[1];}
        CUDA_HOSTDEV float z() const {return e[2];}
        CUDA_HOSTDEV float r() const {return e[0];}
        CUDA_HOSTDEV float g() const {return e[1];}
        CUDA_HOSTDEV float b() const {return e[2];}

        CUDA_HOSTDEV const Vec3& operator+() const;
        CUDA_HOSTDEV Vec3 operator-() const;

        // Array subscript operator
        // User-defined classes that provide array-like access that allows both reading and writing typically define two overloads for operator[]: const and non-const variants:
        // like in https://en.cppreference.com/w/cpp/language/operators
        CUDA_HOSTDEV float operator[](int i) const;
        CUDA_HOSTDEV float& operator[](int i);

        CUDA_HOSTDEV Vec3& operator=(const Vec3 &v2);
        CUDA_HOSTDEV Vec3& operator+=(const Vec3 &v2);
        CUDA_HOSTDEV Vec3& operator-=(const Vec3 &v2);
        CUDA_HOSTDEV Vec3& operator*=(const Vec3 &v2);
        CUDA_HOSTDEV Vec3& operator/=(const Vec3 &v2);
        CUDA_HOSTDEV Vec3& operator*=(const float x);
        CUDA_HOSTDEV Vec3& operator/=(const float x);

        CUDA_HOSTDEV float length() const;
        CUDA_HOSTDEV float squaredLength() const;
        CUDA_HOSTDEV void  makeUnitVector();

        // The friend declaration appears in a class body and grants a function or another class access to private and protected members of the class where the friend declaration appears.
        // the non-member function operator<< will have access to Y's private members
        friend std::istream& operator>>(std::istream &is, Vec3 &t);
        friend std::ostream& operator <<(std::ostream &ps, const Vec3 &t);

        CUDA_HOSTDEV friend Vec3 operator+(const Vec3 &v1, const Vec3 &v2);
        CUDA_HOSTDEV friend Vec3 operator-(const Vec3 &v1, const Vec3 &v2);
        CUDA_HOSTDEV friend Vec3 operator*(const Vec3 &v1, const Vec3 &v2);
        CUDA_HOSTDEV friend Vec3 operator/(const Vec3 &v1, const Vec3 &v2);

        CUDA_HOSTDEV friend Vec3 operator*(float t, const Vec3 &v);
        CUDA_HOSTDEV friend Vec3 operator*(const Vec3 &v, float t);
        CUDA_HOSTDEV friend Vec3 operator/(const Vec3 &v, float t);
        CUDA_HOSTDEV friend float dot(const Vec3 &v1, const Vec3 &v2);
        CUDA_HOSTDEV friend Vec3 cross(const Vec3 &v1, const Vec3 &v2);

};

// unary operators
inline CUDA_HOSTDEV const Vec3& Vec3::operator+() const
{
    return *this;
}

inline CUDA_HOSTDEV Vec3 Vec3::operator-() const
{
    return Vec3(-e[0], -e[1], -e[2]);
}

inline CUDA_HOSTDEV float Vec3::operator[](int i) const
{
    return (*this).e[i];
}

inline CUDA_HOSTDEV float& Vec3::operator[](int i)
{
    return (*this).e[i];
}

// binary operators
inline CUDA_HOSTDEV Vec3& Vec3::operator=(const Vec3 &v2)
{
    e[0] = v2.e[0];
    e[1] = v2.e[1];
    e[2] = v2.e[2];
    return *this;
}

inline CUDA_HOSTDEV Vec3& Vec3::operator+=(const Vec3 &v2)
{
    e[0] += v2.e[0];
    e[1] += v2.e[1];
    e[2] += v2.e[2];
    return *this;
}

inline CUDA_HOSTDEV Vec3& Vec3::operator-=(const Vec3 &v2)
{
    e[0] -= v2.e[0];
    e[1] -= v2.e[1];
    e[2] -= v2.e[2];
    return *this;
}

inline CUDA_HOSTDEV Vec3& Vec3::operator*=(const Vec3 &v2)
{
    e[0] *= v2.e[0];
    e[1] *= v2.e[1];
    e[2] *= v2.e[2];
    return *this;
}

inline CUDA_HOSTDEV Vec3& Vec3::operator/=(const Vec3 &v2)
{
    e[0] /= v2.e[0];
    e[1] /= v2.e[1];
    e[2] /= v2.e[2];
    return *this;
}

inline CUDA_HOSTDEV Vec3& Vec3::operator*=(const float x)
{
    e[0] *= x;
    e[1] *= x;
    e[2] *= x;
    return *this;
}

inline CUDA_HOSTDEV Vec3& Vec3::operator/=(const float x)
{
    float k = 1.0f / x;
    e[0] *= k;
    e[1] *= k;
    e[2] *= k;
    return *this;
}

inline CUDA_HOSTDEV float Vec3::length() const
{
    return static_cast<float>(sqrt(static_cast<double>(e[0]*e[0]+e[1]*e[1]+e[2]*e[2])));
}

inline CUDA_HOSTDEV float Vec3::squaredLength() const
{
    return e[0]*e[0]+e[1]*e[1]+e[2]*e[2];
}

inline CUDA_HOSTDEV void Vec3::makeUnitVector()
{
    float k = 1.0f / (static_cast<float>(sqrt(static_cast<double>(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]))));
    e[0] *= k;
    e[1] *= k;
    e[2] *= k;
}

inline std::istream& operator>>(std::istream &is, Vec3 &t)
{
    is >> t.e[0] >> t.e[1] >> t.e[2];
    return is;
}

inline std::ostream& operator <<(std::ostream &os, const Vec3 &t)
{
    os << t.e[0] << " " << t.e[1] << " " << t.e[2];
    return os;
}

inline CUDA_HOSTDEV Vec3 operator+(const Vec3 &v1, const Vec3 &v2)
{
    return Vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

inline CUDA_HOSTDEV Vec3 operator-(const Vec3 &v1, const Vec3 &v2)
{
    return Vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

inline CUDA_HOSTDEV Vec3 operator*(const Vec3 &v1, const Vec3 &v2)
{
    return Vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}

inline CUDA_HOSTDEV Vec3 operator/(const Vec3 &v1, const Vec3 &v2)
{
    return Vec3(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}

inline CUDA_HOSTDEV Vec3 operator*(float t, const Vec3 &v)
{
    return Vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline CUDA_HOSTDEV Vec3 operator*(const Vec3 &v, float t)
{
    return Vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline CUDA_HOSTDEV Vec3 operator/(const Vec3 &v, float t)
{
    return Vec3(v.e[0]/t, v.e[1]/t, v.e[2]/t);
}

inline CUDA_HOSTDEV float dot(const Vec3 &v1, const Vec3 &v2)
{
    return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
}

inline CUDA_HOSTDEV Vec3 cross(const Vec3 &v1, const Vec3 &v2)
{
    return Vec3((v1.e[1]*v2.e[2] - v1.e[2]*v2.e[1]),
                (-(v1.e[0]*v2.e[2] - v1.e[2]*v2.e[0])),
                (v1.e[0]*v2.e[1] - v1.e[1]*v2.e[0]));

}

inline CUDA_HOSTDEV Vec3 unitVector(const Vec3 v)
{
    return v / v.length();
}
