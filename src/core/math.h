#pragma once
#include "Eigen/Dense"
#include "core/data.h"

namespace lmcore
{
    typedef Eigen::Vector2f Vec2f;
    typedef Eigen::Vector3f Vec3f;
    typedef Eigen::Vector4f Vec4f;
    typedef Eigen::Matrix3f Mat3f;
    typedef Eigen::Matrix4f Mat4f;

    typedef Eigen::Hyperplane<float,2> Line;
    typedef Eigen::ParametrizedLine<float,2> PLine;
}