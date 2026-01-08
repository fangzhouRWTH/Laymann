#pragma once

#include <string>
#include <vector>
#include <array>

#include "core/math.h"

namespace lmcore
{
    struct BBox
    {
        Vec3f xyz;
        Mat4f transform;
    };

    enum class ERoomType
    {
        LivingRoom = 0,
        Bedroom,
        DiningRoom,
        Kitchen,
        Bathroom,

        ENUM_MAX
    };

    enum class EOpeningType
    {
        Door,
        Window,

        ENUM_MAX
    };

    struct FPPoint
    {
        Vec3f value = {0.0f,0.0f,0.0f};
    };

    struct FPLineSegment
    {
        FPPoint start;
        FPPoint end;
    };

    struct FPGeometry
    {
        std::vector<FPPoint> points;
    };

    struct FPRoom
    {
        std::string name;
        ERoomType type;
        std::vector<FPGeometry> geometries;
    };

    struct FPConnection
    {
        int32_t first = -1;
        int32_t second = -1;

        bool out = false;
    };

    struct FPOpening
    {
        std::string name;
        EOpeningType type;
        FPPoint position;
        BBox bounding;
        FPConnection connection;
    };

    struct FPData
    {
        
    };

    struct FloorPlan
    {
        std::vector<FPRoom> rooms;
        std::vector<FPOpening> openings;
        FPData data;
    };

    struct PosColorVertex
    {
        float x, y, z;
        float nx, ny, nz;
        float r, g, b, a;
    };

    struct RenderObject
    {
        std::vector<PosColorVertex> vertices;
        //atm we dont use index draw
    };

    struct StaticStructure
    {
        RenderObject rObj;
        Mat4f transform;
        std::vector<BBox> bboxes;
    };

    struct Walls : public StaticStructure
    {

    };
}