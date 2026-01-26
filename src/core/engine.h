#pragma once
#include <chrono>
#include <thread>

namespace lmcore
{
    class Clock
    {
    public:
        using TimePoint = std::chrono::high_resolution_clock::time_point;
        using Duration = std::chrono::duration<double>;

        Clock()
        {
            reset();
        }

        // 重置时钟（通常在开始新模拟或重新开始时调用）
        void reset()
        {
            m_startTime = std::chrono::high_resolution_clock::now();
            m_lastUpdateTime = m_startTime;
            m_accumulatedTime = 0.0;
            m_isPaused = false;
        }

        // 暂停/恢复时钟
        void togglePause()
        {
            m_isPaused = !m_isPaused;
            if (!m_isPaused)
            {
                // 恢复时重置 lastUpdateTime 以忽略暂停期间的时间
                m_lastUpdateTime = std::chrono::high_resolution_clock::now();
            }
        }

        void pause() { m_isPaused = true; }
        void resume()
        {
            if (m_isPaused)
            {
                m_isPaused = false;
                m_lastUpdateTime = std::chrono::high_resolution_clock::now();
            }
        }

        bool isPaused() const { return m_isPaused; }

        // 更新时钟并返回自上次更新以来经过的时间（delta time）
        double tick()
        {
            if (m_isPaused)
            {
                m_deltaTime = 0.0;
                return m_deltaTime;
            }

            TimePoint now = std::chrono::high_resolution_clock::now();
            m_deltaTime = std::chrono::duration_cast<Duration>(now - m_lastUpdateTime).count();
            m_lastUpdateTime = now;

            // 可选：限制最大 delta（防止长时间卡顿后跳帧过大）
            constexpr double MAX_DELTA = 1.0 / 10.0; // 例如限制为最多 10 FPS 的步长
            if (m_deltaTime > MAX_DELTA)
            {
                m_deltaTime = MAX_DELTA;
            }

            m_accumulatedTime += m_deltaTime;
            return m_deltaTime;
        }

        // 获取自时钟启动以来经过的总时间（秒）
        double getElapsedTime() const
        {
            if (m_isPaused)
            {
                return m_accumulatedTime;
            }
            TimePoint now = std::chrono::high_resolution_clock::now();
            double additional = std::chrono::duration_cast<Duration>(now - m_lastUpdateTime).count();
            return m_accumulatedTime + additional;
        }

        // 获取上一帧的 delta time（秒）
        double getDeltaTime() const
        {
            return m_deltaTime;
        }

        // 睡眠剩余帧时间（用于固定帧率，如 60 FPS）
        void sleepUntilNextFrame(double targetFrameTime)
        {
            if (m_deltaTime < targetFrameTime)
            {
                double sleepTime = targetFrameTime - m_deltaTime;
                if (sleepTime > 0.0)
                {
                    std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
                }
            }
        }

    private:
        TimePoint m_startTime;
        TimePoint m_lastUpdateTime;
        double m_accumulatedTime = 0.0;
        double m_deltaTime = 0.0;
        bool m_isPaused = false;
    };

    class Engine
    {
    public:
        Engine(){}
        void Update();
    private:
        Clock mClock;
        
    };
}