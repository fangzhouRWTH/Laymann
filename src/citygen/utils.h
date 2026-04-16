#pragma once
#include <cmath>
#include "citygen/type.h"

namespace lmcore
{
    inline void f_normalize_2d_fast(float &x, float &y)
    {
        float lenSq = x * x + y * y;
        float invLen = 1.0f / std::sqrt(lenSq); // 可替换为 rsqrt
        x *= invLen;
        y *= invLen;
    }

    inline float f_dot_2d(const float &a_x, const float &a_y, const float &b_x, const float &b_y)
    {
        return a_x * b_x + a_y * b_y;
    }

    inline constexpr float kEps = 1e-6f;

    inline float cross2D(const GVec3f &a, const GVec3f &b, const GVec3f &c)
    {
        // (b - a) x (c - a) 的 z 分量
        const float abx = b.x - a.x;
        const float aby = b.y - a.y;
        const float acx = c.x - a.x;
        const float acy = c.y - a.y;
        return abx * acy - aby * acx;
    }

    inline bool inRange(float v, float minv, float maxv)
    {
        return v >= minv - kEps && v <= maxv + kEps;
    }

    inline bool onSegment2D(const GVec3f &a, const GVec3f &b, const GVec3f &p)
    {
        if (std::fabs(cross2D(a, b, p)) > kEps)
            return false;

        return inRange(p.x, std::min(a.x, b.x), std::max(a.x, b.x)) &&
               inRange(p.y, std::min(a.y, b.y), std::max(a.y, b.y));
    }

    inline bool bboxOverlap2D(const GVec3f &a, const GVec3f &b,
                              const GVec3f &c, const GVec3f &d)
    {
        if (std::max(a.x, b.x) < std::min(c.x, d.x) - kEps)
            return false;
        if (std::max(c.x, d.x) < std::min(a.x, b.x) - kEps)
            return false;
        if (std::max(a.y, b.y) < std::min(c.y, d.y) - kEps)
            return false;
        if (std::max(c.y, d.y) < std::min(a.y, b.y) - kEps)
            return false;
        return true;
    }

    inline int sign(float v)
    {
        if (v > kEps)
            return 1;
        if (v < -kEps)
            return -1;
        return 0;
    }

    inline bool segmentsIntersect2D(const GVec3f &a, const GVec3f &b,
                                    const GVec3f &c, const GVec3f &d)
    {
        // 1. 快速排斥
        if (!bboxOverlap2D(a, b, c, d))
            return false;

        const float c1 = cross2D(a, b, c);
        const float c2 = cross2D(a, b, d);
        const float c3 = cross2D(c, d, a);
        const float c4 = cross2D(c, d, b);

        const int s1 = sign(c1);
        const int s2 = sign(c2);
        const int s3 = sign(c3);
        const int s4 = sign(c4);

        // 2. 严格相交
        if ((s1 * s2 < 0) && (s3 * s4 < 0))
            return true;

        // 3. 处理共线 / 端点接触
        if (s1 == 0 && onSegment2D(a, b, c))
            return true;
        if (s2 == 0 && onSegment2D(a, b, d))
            return true;
        if (s3 == 0 && onSegment2D(c, d, a))
            return true;
        if (s4 == 0 && onSegment2D(c, d, b))
            return true;

        return false;
    }

    inline bool segmentsProperlyIntersect2D(const GVec3f &a, const GVec3f &b,
                                            const GVec3f &c, const GVec3f &d)
    {
        const float c1 = cross2D(a, b, c);
        const float c2 = cross2D(a, b, d);
        const float c3 = cross2D(c, d, a);
        const float c4 = cross2D(c, d, b);

        return (sign(c1) * sign(c2) < 0) && (sign(c3) * sign(c4) < 0);
    }

    inline float distanceSq2D(const GVec2f &a, const GVec2f &b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    inline float distanceSq3D(const GVec3f &a, const GVec3f &b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }
}