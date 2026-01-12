#pragma once
#include "core/math.h"
#include "core/data.h"
#include "Eigen/Geometry"

#include <stdint.h>
#include <algorithm>

namespace lmcore{
    Vec3f get_line_direction(const FPLineSegment & line)
    {
        Vec3f dir = line.start.value - line.end.value;
        return dir;
    }

    // Function to find the intersection point if it exists
    bool is_point_on_segment(const Vec2f & p0,const Vec2f p1, const Vec2f & pi)
    {
        PLine _pfirst = PLine::Through(p0,p1);
        float _sqrl_first = (p1-p0).squaredNorm();
        Vec2f _proj_first = _pfirst.projection(pi);
        Vec2f _dis_first = _proj_first - _pfirst.origin();
        bool _is_on_first = _proj_first.isApprox(pi) &&
            _dis_first.squaredNorm() <= _sqrl_first &&
            _dis_first.dot(_pfirst.direction())>=0.f;
        return _is_on_first;
    }

    struct IntersectionResult
    {
        bool hasIntersection = false;
        std::vector<FPPoint> intersectionPoints;
        std::vector<FPLineSegment> newSegments;
        std::vector<uint32_t> firstIndices;
        std::vector<uint32_t> secondIndices;
    };

    IntersectionResult find_segment_intersection_xy(const FPLineSegment & first, const FPLineSegment & second) {
        Vec2f p0 = {first.start.value.x(),first.start.value.y()};
        Vec2f p1 = {first.end.value.x(),first.end.value.y()};
        Vec2f q0 = {second.start.value.x(),second.start.value.y()};
        Vec2f q1 = {second.end.value.x(),second.end.value.y()};

        Line _first = Line::Through(p0,p1);
        Line _second = Line::Through(q0,q1);
        Vec2f _prob_is = _first.intersection(_second);

        bool on_first = is_point_on_segment(p0,p1,_prob_is);
        bool on_second = is_point_on_segment(q0,q1,_prob_is);
        bool is_parallel = (p1-p0).dot(q0-q1);

        IntersectionResult res;
        Vec3f origin_intersec = {_prob_is.x(),_prob_is.y(),first.start.value.z()};
        
        if(on_first && on_second && !is_parallel)
        {
            res.hasIntersection = true;
            res.intersectionPoints.push_back(lmcore::FPPoint{.value=origin_intersec});
            lmcore::FPLineSegment f_se1 = {.start = first.start,.end = lmcore::FPPoint{.value = origin_intersec}};
            lmcore::FPLineSegment f_se2 = {.start = lmcore::FPPoint{.value = origin_intersec},.end = first.end};
            lmcore::FPLineSegment s_se1 = {.start = second.start,.end = lmcore::FPPoint{.value = origin_intersec}};
            lmcore::FPLineSegment s_se2 = {.start = lmcore::FPPoint{.value = origin_intersec},.end = second.end};
            res.newSegments = {f_se1,f_se2,s_se1,s_se2};
            res.firstIndices = std::vector<uint32_t>{0,1};
            res.secondIndices = std::vector<uint32_t>{2,3};
            return res;
        }

        if((on_first || on_second) && is_parallel)
        {
            // overlap
            float lp0 = p0.norm();
            float lp1 = p1.norm();
            float lq0 = q0.norm();
            float lq1 = q1.norm();
            res.hasIntersection = false;

            if((lp0>lq0&&lp0>lq1&&lp1>lq0&&lp1>lq1)||(lp0<lq0&&lp0<lq1&&lp1<lq0&&lp1<lq1))
            {
                //res.hasIntersection = true;
                std::vector<Vec2f> points = {p0,p1,q0,q1};
                std::vector<float> lengths = {p0.norm(),p1.norm(),q0.norm(),q1.norm()};
                std::vector<uint32_t> indices = {0,1,2,3};

                std::sort(indices.begin(),indices.end(),[&lengths](float i1,float i2){
                    return lengths[i1] < lengths[i2];
                });

                uint32_t i0;
                uint32_t i1;
                uint32_t j0;
                uint32_t j1;

                for(int i = 0; i < 3; i++)
                {
                    if(indices[i]==0)
                        i0 = i;
                    if(indices[i]==1)
                        i1 = i;
                    if(indices[i]==2)
                        j0 = i;
                    if(indices[i]==3)
                        j1 = i;
                }

                uint32_t dis_i = uint32_t(abs(int(i0)-int(i1)));
                uint32_t dis_j = uint32_t(abs(int(j0)-int(j1)));

                lmcore::FPPoint a = {.value = Vec3f(points[indices[0]],};
                lmcore::FPPoint b = {.value = points[indices[0]]};
                lmcore::FPPoint c = {.value = points[indices[0]]};
                lmcore::FPPoint d = {.value = points[indices[0]]};

                if(dis_i == 1 && dis_j == 1)
                {
                    res.newSegments = {first,second};
                    res.firstIndices = {0};
                    res.secondIndices = {1};
                    res.intersectionPoints = {lmcore::FPPoint{.value = points[indices[1]]}};
                    return res;
                }

                if(dis_i == 2 && dis_j == 2)
                {
                    if(indices[0]==0 || indices[0]==1)
                    {
                        res.intersectionPoints = {b,c};
                        res.newSegments = {
                            lmcore::FPLineSegment{.start = a,.end = b},
                            lmcore::FPLineSegment{.start = b,.end = c},
                            lmcore::FPLineSegment{.start = c,.end = d}
                        };
                        res.firstIndices = {0,1};
                        res.secondIndices = {1,2};
                    }
                    else
                    {
                        res.intersectionPoints = {b,c};
                        res.newSegments = {
                            lmcore::FPLineSegment{.start = a,.end = b},
                            lmcore::FPLineSegment{.start = b,.end = c},
                            lmcore::FPLineSegment{.start = c,.end = d}
                        };
                        res.firstIndices = {1,2};
                        res.secondIndices = {0,1};
                    }
                }

                if(dis_i == 3)
                {
                    res.intersectionPoints = {b,c};
                    res.newSegments = {
                        lmcore::FPLineSegment{.start = a,.end = b},
                        lmcore::FPLineSegment{.start = b,.end = c},
                        lmcore::FPLineSegment{.start = c,.end = d}
                    };
                    res.firstIndices = {0,1,2};
                    res.secondIndices = {1};
                }
                else if(dis_j == 3)
                {
                    res.intersectionPoints = {b,c};
                    res.newSegments = {
                        lmcore::FPLineSegment{.start = a,.end = b},
                        lmcore::FPLineSegment{.start = b,.end = c},
                        lmcore::FPLineSegment{.start = c,.end = d}
                    };
                    res.firstIndices = {1};
                    res.secondIndices = {0,1,2};
                }

                if(res.newSegments.size()>0)
                {
                    res.hasIntersection = true;
                    std::vector<uint32_t> nfi;
                    std::vector<uint32_t> nsi;
                    for(auto i : res.firstIndices)
                    {
                        auto s = res.newSegments[i];
                        if((s.end.value-s.start.value).norm()>=0.000001f)
                            nfi.push_back(i);
                    }
                    res.firstIndices = nfi;
                    for(auto i : res.secondIndices)
                    {
                        auto s = res.newSegments[i];
                        if((s.end.value-s.start.value).norm()>=0.000001f)
                            nsi.push_back(i);
                    }
                    res.secondIndices = nsi;
                }

                return res;
            }

            return res;
        }

        //there is no intersec
        res.hasIntersection = false;
        return res;
    }

    // std::vector<lmcore::FPLineSegment> intersect_lines(const FPLineSegment & first, const FPLineSegment & second)
    // {

    // }
}