#pragma once
#include "core/math.h"
#include "core/data.h"
#include "Eigen/Geometry"

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

    bool find_segment_intersection_xy(const FPLineSegment & first, const FPLineSegment & second, FPPoint& intersectionPoint) {
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

        if(on_first && on_second && !is_parallel)
        {
            // there is one intersec
            return true;
        }

        if(on_first && on_second && is_parallel)
        {
            // overlap
            return true;
        }

        //there is no intersec
        return false;
    }

    // std::vector<lmcore::FPLineSegment> intersect_lines(const FPLineSegment & first, const FPLineSegment & second)
    // {

    // }
}