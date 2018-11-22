#pragma once

#include "linmath.h"
#include "array.h"
#include "buffer.h"

enum Shape : uint8_t
{
    Sphere = 0,
    Box,
    ShapeCount
};

enum Blend : uint8_t
{
    Add = 0,
    Sub,
    SmoothAdd,
    SmoothSub,
    BlendCount
};

struct maphit 
{
    uint32_t    id;
    float       distance;
};

inline float operator-(maphit a, maphit b)
{
    return a.distance - b.distance;
}

inline float operator-(float a, maphit b)
{
    return a - b.distance;
}

inline float operator-(maphit a, float b)
{
    return a.distance - b;
}

struct CSG
{
    vec3    center;
    vec3    size;
    float   smoothness;
    Shape   shape;
    Blend   blend;

    inline float Sphere(const vec3& p) const 
    {
        return glm::distance(center, p) - size.x;
    }
    inline float Box(const vec3& p) const 
    {
        vec3 d = glm::abs(p - center) - size;
        return glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0f) + 
            glm::length(glm::max(d, vec3(0.0f)));
    }
    inline maphit Add(maphit a, maphit b) const
    {
        return (a.distance < b.distance) ? a : b;
    }
    inline maphit Sub(maphit a, maphit b) const
    {
        b.distance = -b.distance;
        return (a.distance > b.distance) ? a : b;
    }
    inline maphit SmoothAdd(maphit a, maphit b) const
    {
        float e = glm::max(smoothness - glm::abs(a.distance - b.distance), 0.0f);
        float dis = glm::min(a.distance, b.distance) - e * e * 0.25f / smoothness;
        return 
        {
            a.distance < b.distance ? a.id : b.id, 
            dis
        };
    }
    inline maphit SmoothSub(maphit a, maphit b) const
    {
        a.distance = -a.distance;
        maphit m = SmoothAdd(a, b);
        m.distance = -m.distance;
        return m;
    }
    inline float Distance(const vec3& p) const 
    {
        switch(shape)
        {
            default:
            case Shape::Sphere: return Sphere(p);
            case Shape::Box: return Box(p);
        }
    }
    inline maphit Blend(maphit a, maphit b) const 
    {
        switch(blend)
        {
            default:
            case Blend::Add: return Add(a, b);
            case Blend::Sub: return Sub(a, b);
            case Blend::SmoothAdd: return SmoothAdd(a, b);
            case Blend::SmoothSub: return SmoothSub(a, b);
        }
    }
    inline vec3 Normal(const vec3& p) const
    {
        constexpr float e = 0.001f;
        return glm::normalize(vec3(
            Distance(p + vec3(e, 0.0f, 0.0f)) - Distance(p - vec3(e, 0.0f, 0.0f)),
            Distance(p + vec3(0.0f, e, 0.0f)) - Distance(p - vec3(0.0f, e, 0.0f)),
            Distance(p + vec3(0.0f, 0.0f, e)) - Distance(p - vec3(0.0f, 0.0f, e))
        ));
    }
};

struct CSGList 
{
    Array<uint32_t> indices;
    
    maphit Map(const vec3& p, const CSG* csgs) const 
    {
        maphit a = { 0xffffffff, FLT_MAX };
        for(uint32_t i : indices)
        {
            maphit b = 
            { 
                i, 
                csgs[i].Distance(p) 
            };
            a = csgs[i].Blend(a, b);
        }
        return a;
    }
    vec3 Normal(const vec3& p, const CSG* csgs) const 
    {
        constexpr float e = 0.001f;
        return glm::normalize(vec3(
            Map(p + vec3(e, 0.0f, 0.0f), csgs) - Map(p - vec3(e, 0.0f, 0.0f), csgs),
            Map(p + vec3(0.0f, e, 0.0f), csgs) - Map(p - vec3(0.0f, e, 0.0f), csgs),
            Map(p + vec3(0.0f, 0.0f, e), csgs) - Map(p - vec3(0.0f, 0.0f, e), csgs)
        ));
    }
};

// returns pitch between points
float CreatePoints(
    const CSGList&  list, 
    const CSG*      csgs, 
    int32_t         max_depth, 
    vec3            center, 
    float           radius, 
    Array<vec3>&    out);

void PointsToCubes(const Array<vec3>& input, float pitch, Array<Vertex>& output);
