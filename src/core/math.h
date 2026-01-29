#pragma once
#include "Eigen/Dense"
#include <iostream>

namespace lmcore
{
    typedef Eigen::Vector2f Vec2f;
    typedef Eigen::Vector3f Vec3f;
    typedef Eigen::Vector4f Vec4f;
    typedef Eigen::Matrix3f Mat3f;
    typedef Eigen::Matrix4f Mat4f;

    typedef Eigen::Isometry3f Iso3f;
    typedef Eigen::Quaternionf Quaternion;

    typedef Eigen::Hyperplane<float, 2> Line;
    typedef Eigen::ParametrizedLine<float, 2> PLine;

    inline float _eps_ = 1e-6;

    inline void printIso4(const std::string &name, const Iso3f &iso)
    {
        const auto &mat = iso.matrix();
        std::cout << name << std::endl;
        std::cout << "[" << mat.col(0)[0] << "," << mat.col(1)[0] << "," << mat.col(2)[0] << "," << mat.col(3)[0] << "]" << std::endl;
        std::cout << "[" << mat.col(0)[1] << "," << mat.col(1)[1] << "," << mat.col(2)[1] << "," << mat.col(3)[1] << "]" << std::endl;
        std::cout << "[" << mat.col(0)[2] << "," << mat.col(1)[2] << "," << mat.col(2)[2] << "," << mat.col(3)[2] << "]" << std::endl;
        std::cout << "[" << mat.col(0)[3] << "," << mat.col(1)[3] << "," << mat.col(2)[3] << "," << mat.col(3)[3] << "]" << std::endl;
    }

    inline void printMat4(const std::string &name, const Mat4f &mat)
    {
        std::cout << name << std::endl;
        std::cout << "[" << mat.col(0)[0] << "," << mat.col(1)[0] << "," << mat.col(2)[0] << "," << mat.col(3)[0] << "]" << std::endl;
        std::cout << "[" << mat.col(0)[1] << "," << mat.col(1)[1] << "," << mat.col(2)[1] << "," << mat.col(3)[1] << "]" << std::endl;
        std::cout << "[" << mat.col(0)[2] << "," << mat.col(1)[2] << "," << mat.col(2)[2] << "," << mat.col(3)[2] << "]" << std::endl;
        std::cout << "[" << mat.col(0)[3] << "," << mat.col(1)[3] << "," << mat.col(2)[3] << "," << mat.col(3)[3] << "]" << std::endl;
    }

}