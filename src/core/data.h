#pragma once

#include <string>
#include <vector>
#include <array>

#include "core/math.h"

namespace lmcore
{
    struct PosColorVertex
    {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
        float nx = 0.f;
        float ny = 0.f;
        float nz = 1.f;
        float r = 1.f;
        float g = 1.f;
        float b = 1.f;
        float a = 1.f;
    };

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

        bool operator==(const FPPoint & other) const
        {
            return value == other.value;
        }
    };

    struct FPLineSegment
    {
        FPPoint start;
        FPPoint end;

        bool operator==(const FPLineSegment & other) const
        {
            return ((start==other.start)&&(end==other.end))||((start==other.end)&&(end==other.start));
        }
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
        std::vector<uint32_t> wallIndices;
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

        FPLineSegment segment;
        float low = 0.f;
        float high = 2.4f;
    };

    struct FPWallFace
    {
        std::vector<PosColorVertex> shape;
        lmcore::Vec3f normal;
    };

    struct FPSolidifiedWallGeoData
    {
        std::vector<PosColorVertex> baseForm;
        std::vector<FPWallFace> faces;
        std::vector<PosColorVertex> bands;
    };

    struct FPWallSegment
    {
        FPLineSegment value;
    
        float override_height = 0.f;
        float override_width = 0.f;
    
        //temp
        std::vector<uint32_t> opening_indices;
        FPSolidifiedWallGeoData data;
    };

    struct FPData
    {
        float global_wall_height = 3.f;
        float global_wall_thickness = 0.2;
        float global_floor_thickness = 0.2;
        float global_ceiling_thickness = 0.2;

        float global_door_height = 2.4f;
        float global_window_bottom_height = 1.2f;
        float global_window_top_height = 2.4f;
    };

    struct FloorPlan
    {
        std::vector<FPRoom> rooms;
        std::vector<FPOpening> openings;
        std::vector<FPWallSegment> walls;
        FPData data;
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