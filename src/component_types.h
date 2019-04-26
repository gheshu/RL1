#pragma once

#include "hlsl_types.h"
#include "array.h"
#include "bitfield.h"

struct Position
{
    float3 position;
};

struct Orientation
{
    float4 orientation;
};

struct Scale
{
    float3 scale;
};

struct Velocity
{
    float3 velocity;
};

struct Mass
{
    f32 mass;
};

struct AABB
{
    float3 lo;
    float3 hi;
};

struct Sphere
{
    float3  center;
    f32     radius;
};

struct Plane
{
    float3  normal;
    f32     distance;
};

struct Name
{
    char str[64];
};

struct NameHash
{
    u64 hash;
};

struct ItemID
{
    u32 value;
};

struct Inventory
{
    Array<ItemID> ids;
};

struct Health
{
    f32 value;
};

// ----------------------------------------------------------------------------

enum ComponentType : u32
{
    CT_Position = 0,
    CT_Orientation,
    CT_Scale,
    CT_Velocity,
    CT_Mass,
    CT_AABB,
    CT_Sphere,
    CT_Plane,
    CT_Name,
    CT_NameHash,
    CT_Inventory,
    CT_Health,

    CT_Count
};

using ComponentData = Slice<u8>;
using ComponentFlags = BitField<CT_Count>;

// ----------------------------------------------------------------------------

static constexpr usize sc_ComponentSize[] =
{
    sizeof(Position),
    sizeof(Orientation),
    sizeof(Scale),
    sizeof(Velocity),
    sizeof(Mass),
    sizeof(AABB),
    sizeof(Sphere),
    sizeof(Plane),
    sizeof(Name),
    sizeof(NameHash),
    sizeof(Inventory),
    sizeof(Health),
};
CountAssert(sc_ComponentSize, CT_Count);

// ----------------------------------------------------------------------------

template<typename T>
inline constexpr ComponentType GetComponentType();

#define GCT(T, id) \
    template<> \
    inline constexpr ComponentType GetComponentType<T>() { return id; }

GCT(Position, CT_Position)
GCT(Orientation, CT_Orientation)
GCT(Scale, CT_Scale)
GCT(Velocity, CT_Velocity)
GCT(Mass, CT_Mass)
GCT(AABB, CT_AABB)
GCT(Sphere, CT_Sphere)
GCT(Plane, CT_Plane)
GCT(NameHash, CT_NameHash)
GCT(Name, CT_Name)
GCT(Inventory, CT_Inventory)
GCT(Health, CT_Health)

#undef GCT
