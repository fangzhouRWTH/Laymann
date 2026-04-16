#pragma once

#include <vector>
#include <cmath>

#include "citygen/type.h"

namespace lmcore
{
    inline GVec2f operator+(const GVec2f &a, const GVec2f &b) { return {a.x + b.x, a.y + b.y}; }
    inline GVec2f operator-(const GVec2f &a, const GVec2f &b) { return {a.x - b.x, a.y - b.y}; }
    inline GVec2f operator*(const GVec2f &v, float s) { return {v.x * s, v.y * s}; }
    inline GVec2f operator/(const GVec2f &v, float s) { return {v.x / s, v.y / s}; }

    inline float dot(const GVec2f &a, const GVec2f &b)
    {
        return a.x * b.x + a.y * b.y;
    }

    inline float lengthSq(const GVec2f &v)
    {
        return dot(v, v);
    }

    inline GVec2f normalize(const GVec2f &v)
    {
        float ls = lengthSq(v);
        if (ls > 1e-12f)
        {
            float invLen = 1.0f / std::sqrt(ls);
            return v * invLen;
        }
        return {0.0f, 0.0f};
    }

    inline GVec2f perp(const GVec2f &v)
    {
        return {-v.y, v.x};
    }

    struct StripePoint
    {
        GVec2f left;
        GVec2f right;
    };

    std::vector<StripePoint> BuildStripePoints(const std::vector<GVec2f> &pts, float halfWidth)
    {
        std::vector<StripePoint> out;
        const size_t n = pts.size();
        if (n < 2)
            return out;

        out.resize(n);

        // start
        {
            GVec2f d = normalize(pts[1] - pts[0]);
            GVec2f nrm = perp(d);
            out[0].left = pts[0] + nrm * halfWidth;
            out[0].right = pts[0] - nrm * halfWidth;
        }

        // middle
        for (size_t i = 1; i + 1 < n; ++i)
        {
            GVec2f d0 = normalize(pts[i] - pts[i - 1]);
            GVec2f d1 = normalize(pts[i + 1] - pts[i]);

            GVec2f n0 = perp(d0);
            GVec2f n1 = perp(d1);

            GVec2f m = n0 + n1;
            float mLenSq = lengthSq(m);

            // 处理接近180度或退化情况
            if (mLenSq < 1e-12f)
            {
                out[i].left = pts[i] + n1 * halfWidth;
                out[i].right = pts[i] - n1 * halfWidth;
                continue;
            }

            m = normalize(m);

            float denom = dot(m, n1);
            if (std::fabs(denom) < 1e-6f)
            {
                out[i].left = pts[i] + n1 * halfWidth;
                out[i].right = pts[i] - n1 * halfWidth;
                continue;
            }

            float scale = halfWidth / denom;
            out[i].left = pts[i] + m * scale;
            out[i].right = pts[i] - m * scale;
        }

        // end
        {
            GVec2f d = normalize(pts[n - 1] - pts[n - 2]);
            GVec2f nrm = perp(d);
            out[n - 1].left = pts[n - 1] + nrm * halfWidth;
            out[n - 1].right = pts[n - 1] - nrm * halfWidth;
        }

        return out;
    }
}