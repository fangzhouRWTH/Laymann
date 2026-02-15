#pragma once
#include "Eigen/Dense"
#include <iostream>

namespace lmcore
{
    typedef uint8_t byte;

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

    template <typename T>
    T min(T a, T b)
    {
        return a < b ? a : b;
    }

    template <typename T>
    T max(T a, T b)
    {
        return a > b ? a : b;
    }

    inline bool pointInTriangle(const Vec2f A, const Vec2f B, const Vec2f C, const Vec2f P)
    {
        float v0x = C.x() - A.x(), v0y = C.y() - A.y();
        float v1x = B.x() - A.x(), v1y = B.y() - A.y();
        float v2x = P.x() - A.x(), v2y = P.y() - A.y();

        float dot00 = v0x * v0x + v0y * v0y;
        float dot01 = v0x * v1x + v0y * v1y;
        float dot02 = v0x * v2x + v0y * v2y;
        float dot11 = v1x * v1x + v1y * v1y;
        float dot12 = v1x * v2x + v1y * v2y;

        float invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        // u >= 0, v >= 0, u + v <= 1  => w = 1 - u - v >= 0
        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }

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