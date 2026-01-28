#pragma once
#include <cstdint>
#include <vector>
#include <cassert>
#include <type_traits>
#include "core/renderer.h"
#include "core/simulator.h"

// NOT REALLY AN ECS SYSTEM (TODO, BUT NOT NECESSARY)

namespace lmcore
{
    typedef uint32_t EntityHandle;
    typedef uint32_t ComponentHandle;

    inline uint32_t k_invalid_handle = 0xFFFFFFFF;

    struct RenderComponent
    {
        bool isValid = true;
        RenderObjectHandle handle;
        std::string program;
        bool line = false;
    };

    struct PhysicalComponent
    {
        bool isValid = true;
        PhysicalObjectHandle handle;
    };

    struct Entity
    {
        bool dead = false;
        bool active = true;
        Iso3f iso = Iso3f::Identity();
        ComponentHandle renderComponentHandle = k_invalid_handle;
        ComponentHandle physicalComponentHandle = k_invalid_handle;
    };

    struct EntityPool
    {
        std::vector<Entity> entities;
    };

    struct ISystem;

    class EntityManager
    {
        friend class ISystem;

    public:
        struct EntityBuilder
        {
            EntityManager *manager = nullptr;
            EntityHandle handle = k_invalid_handle;
            Entity *entity;

            EntityBuilder add(RenderComponent rc)
            {
                assert(handle != k_invalid_handle);
                assert(entity->renderComponentHandle == k_invalid_handle);
                ComponentHandle h = manager->mRenderComponentPool.size();
                manager->mRenderComponentPool.push_back(rc);
                entity->renderComponentHandle = h;
                return *this;
            }

            EntityBuilder add(PhysicalComponent pc)
            {
                assert(handle != k_invalid_handle);
                assert(entity->physicalComponentHandle == k_invalid_handle);
                ComponentHandle h = manager->mPhysicalComponentPool.size();
                manager->mPhysicalComponentPool.push_back(pc);
                entity->physicalComponentHandle = h;
                return *this;
            }

            EntityHandle end()
            {
                assert(handle != k_invalid_handle);
                return handle;
            }
        };

        EntityBuilder RegisterEntity()
        {
            auto i = EntityHandle(mPool.entities.size());
            auto e = &(mPool.entities.emplace_back());
            return EntityBuilder{.manager = this, .handle = i, .entity = e};
        }

        void AddPhysicalEngine(std::shared_ptr<Simulator> simulator)
        {
            mSimulator = simulator;
        }

        void AddRenderEngine(std::shared_ptr<Renderer> renderer)
        {
            mRenderer = renderer;
        }

        template <typename T>
        void GatherObjects(std::vector<T> &objs)
        {
            static_assert(std::is_same_v<T, FrameObject>);
            for (auto &e : mPool.entities)
            {
                bool r = (e.renderComponentHandle != k_invalid_handle) &&
                         (e.active) &&
                         (!e.dead);
                if (!r)
                    continue;

                FrameObject fo;
                auto &ro = mRenderComponentPool[e.renderComponentHandle];
                fo.h = ro.handle;
                fo.line = ro.line;
                fo.p = ro.program;
                fo.transform = e.iso;
                objs.push_back(fo);
            }
        }

    private:
        EntityPool mPool;
        std::vector<RenderComponent> mRenderComponentPool;
        std::vector<PhysicalComponent> mPhysicalComponentPool;

        std::shared_ptr<Simulator> mSimulator = nullptr;
        std::shared_ptr<Renderer> mRenderer = nullptr;
    };

    struct ISystem
    {
        virtual void Update(EntityManager *manager) = 0;

    protected:
        EntityPool &get_entities_pool(EntityManager *manager)
        {
            return manager->mPool;
        }

        std::vector<RenderComponent> &get_render_component_array(EntityManager *manager)
        {
            return manager->mRenderComponentPool;
        }

        std::vector<PhysicalComponent> &get_physical_component_array(EntityManager *manager)
        {
            return manager->mPhysicalComponentPool;
        }

        std::shared_ptr<Simulator> get_simualtor(EntityManager *manager)
        {
            return manager->mSimulator;
        }

        std::shared_ptr<Renderer> get_renderer(EntityManager *manager)
        {
            return manager->mRenderer;
        }
    };

    struct PositionUpdater : public ISystem
    {
        virtual void Update(EntityManager *manager)
        {
            auto &pool = get_entities_pool(manager);
            auto &pcs = get_physical_component_array(manager);
            auto size = pool.entities.size();
            for (auto i = 0; i < size; i++)
            {
                auto &e = pool.entities[i];
                auto ph = e.physicalComponentHandle;
                if (ph == k_invalid_handle || ph >= pcs.size())
                    continue;
                auto &pc = pcs[ph];
                if (!pc.isValid || pc.handle == k_invalid_handle)
                    continue;

                auto phy = get_simualtor(manager);
                auto cubestate = phy->GetPhysicalState(pc.handle);
                e.iso = cubestate.pose;
            }
        }
    };

    class SystemsManager
    {
    public:
        void add(std::shared_ptr<ISystem> sys)
        {
            mSystems.push_back(sys);
        }

        void Update(std::shared_ptr<EntityManager> enm)
        {
            for (auto &s : mSystems)
            {
                s->Update(enm.get());
            }
        }

    private:
        std::vector<std::shared_ptr<ISystem>> mSystems;
    };
}