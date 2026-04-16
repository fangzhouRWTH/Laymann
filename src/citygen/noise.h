#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

class PerlinNoise
{
public:
    explicit PerlinNoise(uint32_t seed = 2025)
    {
        reseed(seed);
    }

    void reseed(uint32_t seed)
    {
        // 构造 0~255 的排列
        std::array<int, 256> perm{};
        std::iota(perm.begin(), perm.end(), 0);

        std::mt19937 rng(seed);
        std::shuffle(perm.begin(), perm.end(), rng);

        // 复制两遍，避免额外取模
        for (int i = 0; i < 256; ++i)
        {
            p_[i] = perm[i];
            p_[256 + i] = perm[i];
        }
    }

    // ----------------------------
    // 1D Perlin Noise
    // 返回值通常在 [-1, 1] 附近
    // ----------------------------
    double noise(double x) const
    {
        int X = fastFloor(x) & 255;
        x -= fastFloor(x);

        double u = fade(x);

        int a = p_[X];
        int b = p_[X + 1];

        double res = lerp(
            grad1(a, x),
            grad1(b, x - 1.0),
            u
        );

        return res;
    }

    // ----------------------------
    // 2D Perlin Noise
    // 返回值通常在 [-1, 1] 附近
    // ----------------------------
    double noise(double x, double y) const
    {
        int X = fastFloor(x) & 255;
        int Y = fastFloor(y) & 255;

        x -= fastFloor(x);
        y -= fastFloor(y);

        double u = fade(x);
        double v = fade(y);

        int aa = p_[p_[X] + Y];
        int ab = p_[p_[X] + Y + 1];
        int ba = p_[p_[X + 1] + Y];
        int bb = p_[p_[X + 1] + Y + 1];

        double x1 = lerp(
            grad2(aa, x, y),
            grad2(ba, x - 1.0, y),
            u
        );

        double x2 = lerp(
            grad2(ab, x, y - 1.0),
            grad2(bb, x - 1.0, y - 1.0),
            u
        );

        return lerp(x1, x2, v);
    }

    // ----------------------------
    // 3D Perlin Noise
    // 返回值通常在 [-1, 1] 附近
    // ----------------------------
    double noise(double x, double y, double z) const
    {
        int X = fastFloor(x) & 255;
        int Y = fastFloor(y) & 255;
        int Z = fastFloor(z) & 255;

        x -= fastFloor(x);
        y -= fastFloor(y);
        z -= fastFloor(z);

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int aaa = p_[p_[p_[X] + Y] + Z];
        int aba = p_[p_[p_[X] + Y + 1] + Z];
        int aab = p_[p_[p_[X] + Y] + Z + 1];
        int abb = p_[p_[p_[X] + Y + 1] + Z + 1];

        int baa = p_[p_[p_[X + 1] + Y] + Z];
        int bba = p_[p_[p_[X + 1] + Y + 1] + Z];
        int bab = p_[p_[p_[X + 1] + Y] + Z + 1];
        int bbb = p_[p_[p_[X + 1] + Y + 1] + Z + 1];

        double x1 = lerp(
            grad3(aaa, x, y, z),
            grad3(baa, x - 1.0, y, z),
            u
        );
        double x2 = lerp(
            grad3(aba, x, y - 1.0, z),
            grad3(bba, x - 1.0, y - 1.0, z),
            u
        );
        double y1 = lerp(x1, x2, v);

        double x3 = lerp(
            grad3(aab, x, y, z - 1.0),
            grad3(bab, x - 1.0, y, z - 1.0),
            u
        );
        double x4 = lerp(
            grad3(abb, x, y - 1.0, z - 1.0),
            grad3(bbb, x - 1.0, y - 1.0, z - 1.0),
            u
        );
        double y2 = lerp(x3, x4, v);

        return lerp(y1, y2, w);
    }

    // ----------------------------
    // fBm / Octave Perlin Noise (2D)
    // 常用于地形、高度图、密度场
    // ----------------------------
    double fbm(double x, double y,
               int octaves = 6,
               double frequency = 1.0,
               double amplitude = 1.0,
               double lacunarity = 2.0,
               double persistence = 0.5) const
    {
        double sum = 0.0;
        double ampSum = 0.0;

        double freq = frequency;
        double amp = amplitude;

        for (int i = 0; i < octaves; ++i)
        {
            sum += noise(x * freq, y * freq) * amp;
            ampSum += amp;

            freq *= lacunarity;
            amp *= persistence;
        }

        if (ampSum > 0.0)
            sum /= ampSum;

        return sum;
    }

    // 归一化到 [0, 1]
    double noise01(double x, double y) const
    {
        return noise(x, y) * 0.5 + 0.5;
    }

    double fbm01(double x, double y,
                 int octaves = 6,
                 double frequency = 1.0,
                 double amplitude = 1.0,
                 double lacunarity = 2.0,
                 double persistence = 0.5) const
    {
        return fbm(x, y, octaves, frequency, amplitude, lacunarity, persistence) * 0.5 + 0.5;
    }

private:
    std::array<int, 512> p_{};

    static int fastFloor(double x)
    {
        return (x >= 0.0) ? static_cast<int>(x) : static_cast<int>(x) - 1;
    }

    // Perlin 的经典平滑函数：6t^5 - 15t^4 + 10t^3
    static double fade(double t)
    {
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    static double lerp(double a, double b, double t)
    {
        return a + t * (b - a);
    }

    // 1D 梯度
    static double grad1(int hash, double x)
    {
        // 只取符号变化
        return (hash & 1) ? x : -x;
    }

    // 2D 梯度
    static double grad2(int hash, double x, double y)
    {
        // 8 个方向的简化形式
        switch (hash & 7)
        {
            case 0: return  x + y;
            case 1: return -x + y;
            case 2: return  x - y;
            case 3: return -x - y;
            case 4: return  x;
            case 5: return -x;
            case 6: return  y;
            case 7: return -y;
            default: return 0.0;
        }
    }

    // 3D 梯度
    static double grad3(int hash, double x, double y, double z)
    {
        // Ken Perlin 经典实现风格
        int h = hash & 15;
        double u = (h < 8) ? x : y;
        double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);

        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }
};

// int main()
// {
//     PerlinNoise perlin(12345);

//     // 示例1：直接采样 2D noise
//     std::cout << "2D noise samples:\n";
//     for (int y = 0; y < 5; ++y)
//     {
//         for (int x = 0; x < 5; ++x)
//         {
//             double nx = x * 0.1;
//             double ny = y * 0.1;
//             std::cout << perlin.noise(nx, ny) << " ";
//         }
//         std::cout << "\n";
//     }

//     std::cout << "\n2D fbm samples:\n";
//     for (int y = 0; y < 5; ++y)
//     {
//         for (int x = 0; x < 5; ++x)
//         {
//             double nx = x * 0.05;
//             double ny = y * 0.05;
//             std::cout << perlin.fbm(nx, ny, 5, 1.0, 1.0, 2.0, 0.5) << " ";
//         }
//         std::cout << "\n";
//     }

//     return 0;
// }