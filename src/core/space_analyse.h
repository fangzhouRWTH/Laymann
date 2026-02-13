#pragma once
#include <vector>
#include <cassert>
#include <stdint.h>
#include "core/math.h"
#include "core/data.h"

namespace lmcore
{
    struct Area2D_XY
    {
        std::vector<Vec3f> triangle_points;
    };

    struct FieldSample
    {
        Vec3f world_pos;
    };

    class DiscreteSpaceField
    {
    public:
        static DiscreteSpaceField Create(Area2D_XY area, float dist = 0.1)
        {
            auto size = area.triangle_points.size();
            assert(size != 0);
            float x_min = area.triangle_points[0].x();
            float x_max = area.triangle_points[0].x();
            float y_min = area.triangle_points[0].y();
            float y_max = area.triangle_points[0].y();
            float z = area.triangle_points[0].z();

            for (auto i = 1; i < size; i++)
            {
                auto &p = area.triangle_points[i];
                x_min = min(x_min, p.x());
                x_max = max(x_max, p.x());
                y_min = min(y_min, p.y());
                y_max = max(y_max, p.y());
            }

            uint32_t x_count = (x_max - x_min) / dist + 1;
            uint32_t y_count = (y_max - y_min) / dist + 1;
            uint32_t estimated_count = x_count * y_count;
            std::vector<Vec3f> descrete_pos;
            descrete_pos.reserve(estimated_count);
            for (auto _x = 0; _x < x_count; _x++)
            {
                for (auto _y = 0; _y < y_count; _y++)
                {
                    Vec3f pos = {_x * dist + x_min, _y * dist + y_min, z};
                    bool inside = false;
                    for (auto _t = 0; _t < size; _t += 3)
                    {
                        Vec3f A = area.triangle_points[_t];
                        Vec3f B = area.triangle_points[_t + 1];
                        Vec3f C = area.triangle_points[_t + 2];

                        inside = pointInTriangle({A.x(), A.y()}, {B.x(), B.y()}, {C.x(), C.y()}, {pos.x(), pos.y()});
                        if (inside)
                            break;
                    }
                    if (inside)
                        descrete_pos.push_back(pos);
                }
            }

            DiscreteSpaceField field;
            field.samples.reserve(descrete_pos.size());
            for (auto p : descrete_pos)
            {
                FieldSample s;
                s.world_pos = p;
                field.samples.push_back(s);
            }

            return std::move(field);
        }

        void GenerateVertices()
        {
            vertices_data.reserve(6u * samples.size());
            for (auto s : samples)
            {
                for (auto v : s_vis_verts)
                {
                    PosColorVertex pv;
                    pv.x = v.x() * sample_vis_scale + s.world_pos.x();
                    pv.y = v.y() * sample_vis_scale + s.world_pos.y();
                    pv.z = v.z() * sample_vis_scale + s.world_pos.z();
                    vertices_data.push_back(pv);
                }
            }
        }

        std::vector<FieldSample> &GetSamples()
        {
            return samples;
        }

        std::vector<PosColorVertex> &GetVertices()
        {
            return vertices_data;
        }

    private:
        // static std::array<PosColorVertex,18u> s_vis_verts;
        static std::array<Vec3f, 6u> s_vis_verts;
        float sample_vis_scale = 0.1;
        std::vector<FieldSample> samples;
        std::vector<PosColorVertex> vertices_data;
    };
}