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

#include "util/ray.h"
#include "util/util.h"
#include "util/randomgenerator.h"

enum CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// fov - field of view
// image is not square => fow is different horizontally and vertically

class Camera
{

    public:
        Vec3 origin;
        Vec3 lowerLeftCorner;
        Vec3 horizontal;
        Vec3 vertical;
        Vec3 u, v, w;
        float lensRadius;
        float time0, time1; // shutter open/close times

        Vec3 lookFrom;
        Vec3 lookAt;

        Vec3 vup;
        float vfov;
        float aspect;
        float aperture;
        float focusDist;

        float halfWidth;
	    float halfHeight;

        CUDA_HOSTDEV Camera():   origin(Vec3(0.0f, 0.0f, 0.0f)),
                                 lowerLeftCorner(Vec3(-2.0f, -1.0f, -1.0f)),
                                 horizontal(Vec3(4.0f, 0.0f, 0.0f)),
                                 vertical(Vec3(0.0f, 2.0f, 0.0f))
        {

        }
        
        // vfov is top to bottom in degrees
        CUDA_HOSTDEV Camera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup,
                float vfov, float aspect)
        {
            float theta = vfov * static_cast<float>(M_PI)/180.0f;
            this->halfHeight = static_cast<float>(tan(static_cast<double>(theta/2.0f)));
            this->halfWidth = aspect * halfHeight;

            this->origin = lookFrom;
            this->w = unitVector(lookFrom - lookAt);
            this->u = unitVector(cross(vup, w));
            this->v = cross(w, u);

            this->lowerLeftCorner = origin - halfWidth*u - halfHeight*v - w;
            this->horizontal = 2.0f*halfWidth*u;
            this->vertical = 2.0f*halfHeight*v;

            this->lookFrom = lookFrom;
            this->lookAt = lookAt;

            this->vup = unitVector(vup);
            this->vfov = vfov;
            this->aspect = aspect;
        }

        // another constructor
        CUDA_HOSTDEV Camera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup,
                float vfov, float aspect, float focusDist,
                float aperture = 0.0f, float t0 = 0.0f, float t1 = 1.0f) :
        Camera(lookFrom, lookAt, vup, vfov, aspect)
        {
            this->time0 = t0;
            this->time1 = t1;

            this->lensRadius = aperture/2.0f;
            this->aperture = aperture;
            this->focusDist = focusDist;

            this->lowerLeftCorner = origin - halfWidth*focusDist*u - halfHeight*focusDist*v - focusDist*w;
            this->horizontal = 2.0f*halfWidth*focusDist*u;
            this->vertical = 2.0f*halfHeight*focusDist*v;
        }

        CUDA_HOSTDEV void update()
        {
            float theta = vfov * static_cast<float>(M_PI) / 180.0f;
            this->halfHeight = static_cast<float>(tan(static_cast<double>(theta / 2.0f)));
            this->halfWidth = aspect * halfHeight;

            this->origin = lookFrom;
            this->w = unitVector(lookFrom - lookAt);
            this->u = unitVector(cross(vup, w));
            this->v = cross(w, u);

            this->lowerLeftCorner = origin - halfWidth*focusDist*u - halfHeight*focusDist*v - focusDist*w;
            this->horizontal = 2.0f*halfWidth*focusDist*u;
            this->vertical = 2.0f*halfHeight*focusDist*v;
        }

        // Spherical coordinate system implementation - rotate the lookFrom location by theta polar angle and phi azimuth angle - keeping the distance 
        CUDA_HOSTDEV void rotate(float theta, float phi)
        {
            float radialDistance = (lookFrom - lookAt).length();
            this->lookFrom = Vec3(
                radialDistance*sinf(theta)*sinf(phi),
                radialDistance*cosf(theta),
                radialDistance*sinf(theta)*cosf(phi)) + lookAt;
            update();
        }

        CUDA_HOSTDEV void zoom(float zoomScale)
        {
            this->vfov += zoomScale;
            // min(max())
            this->vfov = clamp<float>(this->vfov, 0.0f, 180.0f);
            update();
        }

        CUDA_HOSTDEV void translate(CameraMovement direction, float stepScale)
        {
            if (direction == FORWARD)
            {
                lookFrom += this->v * stepScale;
                lookAt += this->v * stepScale;;
            }
            if (direction == BACKWARD)
            {
                lookFrom -= this->v * stepScale;
                lookAt -= this->v * stepScale;
            }
            if (direction == LEFT)
            {
                lookFrom -= this->u * stepScale;
                lookAt -= this->u * stepScale;
            }
            if (direction == RIGHT)
            {
                lookFrom += this->u * stepScale;
                lookAt += this->u * stepScale;
            }
            update();
        }

        CUDA_DEV Ray getRay(RandomGenerator& rng, float s, float t)
        {
            Vec3 rd = lensRadius*rng.randomInUnitSphere();
            Vec3 offset = u * rd.x() + v * rd.y();
            float time = time0 + rng.get1f()*(time1-time0);
            return Ray(origin + offset, lowerLeftCorner + s*horizontal + t*vertical - origin - offset, time);
        }

};
