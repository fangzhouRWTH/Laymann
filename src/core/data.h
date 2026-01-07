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
        Vec4f value;
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

    struct FloorPlan
    {
        std::vector<FPRoom> rooms;
        std::vector<FPOpening> openings;
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
}