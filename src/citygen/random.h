#pragma once

#include <random>

namespace lmcore
{
    class Random
    {
    public:
        Random()
            : mGen(std::random_device{}())
        {
        }

        explicit Random(unsigned int seed)
            : mGen(seed)
        {
        }

        int nextInt(int min, int max)
        {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(mGen);
        }

        float nextFloat(float min, float max)
        {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(mGen);
        }

        int nextSign()
        {
            return (mGen() & 1) ? 1 : -1;
        }

        float nextSignFloat()
        {
            return (mGen() & 1) ? 1.0f : -1.0f;
        }

        float nextGaussian(float mean, float stddev)
        {
            std::normal_distribution<float> dist(mean, stddev);
            return dist(mGen);
        }

        float nextGaussianClamped(float mean, float stddev, float min, float max)
        {
            std::normal_distribution<float> dist(mean, stddev);

            for (int i = 0; i < 10; ++i)
            {
                float v = dist(mGen);
                if (v >= min && v <= max)
                    return v;
            }

            float v = dist(mGen);
            return std::max(min, std::min(max, v));
        }

        float nextGaussianApprox(float min, float max)
        {
            float sum = 0.0f;

            // 3~6 次就够了
            for (int i = 0; i < 4; ++i)
                sum += nextFloat(0.0f, 1.0f);

            float v = sum / 4.0f; // [0,1] 近似正态

            return min + (max - min) * v;
        }

    private:
        std::mt19937 mGen;
    };
}