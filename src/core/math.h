#pragma once
#include "Eigen/Dense"

namespace lmcore
{
    typedef Eigen::Vector2f Vec2f;
    typedef Eigen::Vector3f Vec3f;
    typedef Eigen::Vector4f Vec4f;
    typedef Eigen::Matrix3f Mat3f;
    typedef Eigen::Matrix4f Mat4f;

    typedef Eigen::Isometry3f Iso3f;
    typedef Eigen::Quaternionf Quaternion;

    typedef Eigen::Hyperplane<float,2> Line;
    typedef Eigen::ParametrizedLine<float,2> PLine;

    inline float _eps_ = 1e-6; 
}