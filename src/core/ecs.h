#pragma once
#include <cstdint>
#include <vector>
#include <cassert>
#include <type_traits>
#include "core/component.h"
#include "core/renderer.h"
#include "core/simulator.h"

namespace lmcore
{
    typedef uint32_t EntityHandle;
    typedef uint32_t ComponentHandle;

    inline uint32_t k_invalid_handle = 0xFFFFFFFF;

    struct ECSRegistry;

    struct ECSUpdateContext
    {
        float deltaTime = 0.f;
        uint32_t frameBufferHeight = 0u;
        uint32_t frameBufferWidth = 0u;
        std::shared_ptr<ECSRegistry> registry = nullptr;
    };

    class ECSSystem
    {
    public:
        virtual void PreUpdate(ECSUpdateContext context) {};
        virtual void Update(ECSUpdateContext context) = 0;
        virtual void PostUpdate(ECSUpdateContext context) {}

    private:
    };

    class ECSRenderSystem final : public ECSSystem
    {
    public:
        ECSRenderSystem(std::shared_ptr<Renderer> renderer) : mRenderer(renderer) {};

        virtual void PreUpdate(ECSUpdateContext context);
        virtual void Update(ECSUpdateContext context);
        virtual void PostUpdate(ECSUpdateContext context);
    private:
        std::shared_ptr<Renderer> mRenderer;
    };

    class ECSPhysicalVisualizationSystem final : public ECSSystem
    {
    public:
        ECSPhysicalVisualizationSystem(std::shared_ptr<Renderer> renderer) : mRenderer(renderer) {};
        
        virtual void PreUpdate(ECSUpdateContext context);
        virtual void Update(ECSUpdateContext context);
        virtual void PostUpdate(ECSUpdateContext context);
    private:
        std::shared_ptr<Renderer> mRenderer;
    };

    class ECSPhysicSystem final : public ECSSystem
    {
    public:
        ECSPhysicSystem(std::shared_ptr<Simulator> simulator) : mSimulator(simulator)
        {
        }

        virtual void Update(ECSUpdateContext context);

    private:
        std::shared_ptr<Simulator> mSimulator = nullptr;
    };

    class ECSCameraControlSystem : public ECSSystem
    {
    public:
        ECSCameraControlSystem(std::shared_ptr<Control> ctrl) : ctrlptr(ctrl)
        {
        }

        virtual void Update(ECSUpdateContext context);

    private:
        std::shared_ptr<Control> ctrlptr = nullptr;
    };

    class ECSManager
    {
    public:
        ~ECSManager();
        ECSManager();

        void Init();
        EntityHandle Create();

        void AddSystem(std::shared_ptr<ECSSystem> system);
        void PreUpdateSystems(uint32_t fbw, uint32_t fbh);
        void UpdateSystems(double deltaTime);
        void PostUpdateSystems();

        template <typename Component>
        void Add(EntityHandle handle, Component component);

    private:
        class Impl;

        std::unique_ptr<Impl> impl;
    };
}