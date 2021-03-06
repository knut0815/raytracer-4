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

#include "hitables/hitable.h"

class HitableList: public Hitable
{

    public: 

        Hitable **list;
        int listSize;
        CUDA_DEV HitableList() {}
        CUDA_DEV HitableList(Hitable **l, int n)
        {
            list = l;
            listSize = n;
        }

        CUDA_DEV bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override;
        CUDA_DEV bool boundingBox(float t0, float t1, AABB& box) const override;

};

inline CUDA_DEV bool HitableList::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const
{

    HitRecord tempRec;
    bool hitAnything = false;
    float closestSoFar = tMax;

    for (int i = 0; i < listSize; i++)
    {
        // if the list item was hit
        if (list[i]->hit(r, tMin, closestSoFar, tempRec))
        {
            hitAnything = true;
            closestSoFar = tempRec.time;
            rec = tempRec;
        }
    }

    return hitAnything;

}

inline CUDA_DEV bool HitableList::boundingBox(float t0, float t1, AABB& box) const
{

    if (listSize < 1)
        return false;
    AABB tempBox;
    bool firstTrue = list[0]->boundingBox(t0, t1, tempBox);

    if (!firstTrue)
        return false;
    else
        box = tempBox;
    for (int i = 1; i < listSize; i++)
    {
        if(list[0]->boundingBox(t0, t1, tempBox))
        {
            box = surroundingBox(box, tempBox);
        }
        else
            return false;
    }

    return true;

}
