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

/*
As in Peter Shirley's book:
- diffuse = matte materials
- diffuse objects that don't emit light take on the color of their surroundings
- BUT they modulate that with their own intrinsic colot
- light that reflects off a diffuse surface has its direction randomized
- if we send three rays into a crack between two diffuse surfaces they will each have different random behavior
- rays might be absorbed
- the darker the durface, the more likely absorption is
*/

#pragma once

struct hitRecord;

#include "util/ray.h"
#include "hitables/hitable.h"
#include "util/randomgenerator.h"

class material
{
    public:
        CUDA_DEV virtual bool scatter(RandomGenerator& rng, const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const = 0;
        CUDA_DEV virtual ~material() {}
};

// lambertian (diffuse)
// it can either scatter always and attenuate by its reflectance R
// or it can scatter with no attenuation but absorb the fraction 1-R of the rays
// or MIXED of these two strategies
class lambertian : public material 
{
    vec3 albedo; // the proportion of the incident light or radiation that is reflected by a surface
    public:
        CUDA_DEV lambertian(const vec3& a) : albedo(a) {}

        // diffuse matrials randomly scatter the rays
        CUDA_DEV virtual bool scatter(RandomGenerator& rng, const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const
        {
            vec3 target = rec.point + rec.normal + rng.randomInUnitSphere();
            scattered = ray(rec.point, target - rec.point, rIn.time());
            attenuation = albedo;
            return true;
        }
};

// for smooth metals the ray won't be randomly scattered
// because v points in, we will need a minus sign before the dot product
CUDA_DEV inline vec3 reflect(const vec3& v, const vec3& n)
{
    return v - 2.0f*dot(v,n)*n;
}

class metal: public material
{
    vec3 albedo;
    float fuzz;
    public:
        CUDA_DEV metal(const vec3& a, float f = 0.0f) : albedo(a) {if (f < 1.0f) fuzz = f; else fuzz = 1.0f;}
        CUDA_DEV virtual bool scatter(RandomGenerator& rng, const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const;
};

// metals don't randomly scatter -> they reflect
CUDA_DEV inline bool metal::scatter(RandomGenerator& rng, const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const
{
    vec3 reflected = reflect(unitVector(rIn.direction()), rec.normal);
    scattered = ray(rec.point, reflected + fuzz*rng.randomInUnitSphere(), rIn.time());
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0.0f);
}


// dielectrics: water, glass, diamonds (those are clear materials)
// when a ray hits them, it splits into a reflected ray and a refracted (transmitted) ray
// randomly choosing between reflection and refraction -> 
// only generating one scattered ray per interaction

// refraction is described by Snell's law:
// n sin(theta) = n' sin(theta')
// n and n' are the refractive indices (air = 1, glass = 1.2-1.7, diamond = 2.4)
// total internal reflection
CUDA_DEV inline bool refract(const vec3& v, const vec3& n, float niOverNt, vec3& refracted)
{
    vec3 uv = unitVector(v);
    float dt = dot(uv, n);
    float discriminant = 1.0f - niOverNt*niOverNt*(1.0f - dt*dt);
    if (discriminant > 0.0f)
    {
        refracted = niOverNt*(uv - n*dt) - n*static_cast<float>(sqrt(static_cast<double>(discriminant)));
        return true;
    }
    else
        return false;
}

class dielectric: public material
{
    float refIndex;
    public:
        CUDA_DEV dielectric(float ri) : refIndex(ri) {}
        CUDA_DEV virtual bool scatter(RandomGenerator& rng, const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const;
};

// real glass has reflectivity that varies with angle
// Christophe Schlick's simple qeuation:
CUDA_DEV inline float schlick(float cosine, float refIndex)
{
    float r0 = (1.0f - refIndex) / (1.0f + refIndex);
    r0 = r0*r0;
    return r0 + (1.0f - r0)*static_cast<float>(pow(static_cast<double>((1.0f - cosine)), 5.0));
}

CUDA_DEV inline bool dielectric::scatter(RandomGenerator& rng, const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const
{
    vec3 outWardNormal;
    vec3 reflected = reflect(rIn.direction(), rec.normal);
    float niOverNt;
    // the glass surface absorbs nothing => attenuation = 1
    // erase the blue channel 
    attenuation = vec3(1.0f, 1.0f, 1.0f);
    vec3 refracted;
    float reflectProbability;
    float cosine;

    if (dot(rIn.direction(), rec.normal) > 0.0f)
    {
        outWardNormal = -rec.normal;
        niOverNt = refIndex;
        cosine = refIndex * dot(rIn.direction(), rec.normal) / rIn.direction().length();
    }
    else
    {
        outWardNormal = rec.normal;
        niOverNt = 1.0f / refIndex;
        cosine = -dot(rIn.direction(), rec.normal) / rIn.direction().length();
    }

    if (refract(rIn.direction(), outWardNormal, niOverNt, refracted))
        reflectProbability = schlick(cosine, refIndex);
    else
    {
        scattered = ray(rec.point, reflected, rIn.time());
        reflectProbability = 1.0f;
    }

    if (rng.get1f() < reflectProbability)
        scattered = ray(rec.point, reflected, rIn.time());
    else
        scattered = ray(rec.point, refracted, rIn.time());
    return true;
}
