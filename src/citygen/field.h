#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include <cassert>
#include <cmath>

#include "citygen/type.h"
#include "citygen/utils.h"

namespace lmcore
{
    // TODO ABSTRACT

    const float F_Dirs_16_2f[16][2] =
        {
            {1.000000f, 0.000000f},
            {0.923880f, 0.382683f},
            {0.707107f, 0.707107f},
            {0.382683f, 0.923880f},
            {0.000000f, 1.000000f},
            {-0.382683f, 0.923880f},
            {-0.707107f, 0.707107f},
            {-0.923880f, 0.382683f},
            {-1.000000f, 0.000000f},
            {-0.923880f, -0.382683f},
            {-0.707107f, -0.707107f},
            {-0.382683f, -0.923880f},
            {0.000000f, -1.000000f},
            {0.382683f, -0.923880f},
            {0.707107f, -0.707107f},
            {0.923880f, -0.382683f}};

    class Field1F
    {
    public:
        explicit Field1F(const std::vector<float> &data, uint32_t resWidth, uint32_t resHeight, float width, float height, float scale)
        {
            auto _size = data.size();
            assert(resWidth * resHeight == _size);
            assert(width > 0.f && height > 0.f);
            mResWidth = resWidth;
            mResHeight = resHeight;
            mHalfWidth = width / 2.f;
            mHalfHeight = height / 2.f;
            mScale = scale;
            mData.resize(_size);
            std::copy(data.begin(), data.end(), mData.begin());
        }
        ~Field1F() {}

        float Sample(float posx, float posy)
        {
            float u = (posx / (mHalfWidth * 2.f) + 0.5) * mResWidth;
            float v = (posy / (mHalfHeight * 2.f) + 0.5) * mResHeight;
            
            if (!check_bound(posx, posy))
                return 0.f;

            return sample(u,v);
        }

        GVec3f SampleDxyz(float posx, float posy, float dist, GVec3f guide = {0.0, 0.0, 0.0}, float range = 0.6f)
        {
            if (!check_bound(posx, posy))
                return {0.f, 0.f, 0.f};

            float u = (posx / (mHalfWidth * 2.f) + 0.5) * mResWidth;
            float v = (posy / (mHalfHeight * 2.f) + 0.5) * mResHeight;

            auto center = sample(u, v);

            std::vector<std::pair<uint32_t, float>> ds;
            for (auto i = 0u; i < 16u; i++)
            {
                bool inv = in_view_range_2d(F_Dirs_16_2f[i][0], F_Dirs_16_2f[i][1], guide.x, guide.y, range);
                if (!inv)
                    continue;
                float _x = F_Dirs_16_2f[i][0] * dist;
                float _y = F_Dirs_16_2f[i][1] * dist;
                float d_x = _x + u;
                float d_y = _y + v;
                // if (!check_bound(d_x, d_y))
                //      continue;
                if(d_x < 0.f || d_y < 0.f)
                    continue;
                ds.push_back({i, sample(d_x, d_y)});
            }

            if (ds.empty())
                return {0.f, 0.f, 0.f};

            uint32_t idx = 0;
            uint32_t dsize = ds.size();
            for (uint32_t i = 1; i < dsize; i++)
            {
                //if (std::abs(ds[idx].second - center) < std::abs(ds[i].second - center))
                if (ds[idx].second > ds[i].second)
                {
                    idx = i;
                }
            }

            return {F_Dirs_16_2f[ds[idx].first][0] * dist, F_Dirs_16_2f[ds[idx].first][1] * dist, ds[idx].second};
        }

    private:
        float sample(float u, float v)
        {
            u = std::fmod(u, static_cast<float>(mResWidth));
            v = std::fmod(v, static_cast<float>(mResWidth));

            uint32_t u_low = static_cast<uint32_t>(std::floor(u));
            uint32_t u_high = static_cast<uint32_t>(std::ceil(u));

            uint32_t v_low = static_cast<uint32_t>(std::floor(v));
            uint32_t v_high = static_cast<uint32_t>(std::ceil(v));

            float u_res = u - u_low;
            float v_res = v - v_low;

            float _0 = mData[v_low * mResWidth + u_low];
            float _1 = mData[v_low * mResWidth + u_high];
            float _2 = mData[v_high * mResWidth + u_low];
            float _3 = mData[v_high * mResWidth + u_high];

            float _u0 = _1 * u_res + _0 * (1.0 - u_res);
            float _u1 = _3 * u_res + _2 * (1.0 - u_res);
            float _value = _u1 * v_res + _u0 * (1.0 - v_res);

            return _value * mScale;
        }

        bool in_view_range_2d(const float vx, const float vy, const float gx, const float gy, const float range)
        {
            if (gx == 0.f &&
                gy == 0.f)
                return true;

            float v_x = vx;
            float v_y = vy;
            float g_x = gx;
            float g_y = gy;

            f_normalize_2d_fast(v_x, v_y);
            f_normalize_2d_fast(g_x, g_y);

            float r = f_dot_2d(v_x, v_y, g_x, g_y);
            float rr = std::cos(range);

            return r >= rr;
        }

        bool check_bound(const float u, const float v)
        {
            bool res = u >= -mHalfWidth;
            res &= u <= mHalfWidth;
            res &= v >= -mHalfHeight;
            res &= v <= mHalfHeight;

            return res;
        }

    private:
        std::vector<float> mData;
        uint32_t mResWidth;
        uint32_t mResHeight;
        float mHalfWidth;
        float mHalfHeight;
        float mScale;
    };
}