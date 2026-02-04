#include "ecs.h"

#include "entt/entt.hpp"

namespace lmcore
{
    struct position
    {
        float x;
        float y;
    };

    struct velocity
    {
        float dx;
        float dy;
    };

    struct ECSRegistry
    {
        entt::registry registry;
    };

    struct EntityMapper
    {
        EntityHandle add(entt::entity ent)
        {
            EntityHandle h = mapper.size();
            if (h == k_invalid_handle)
                return h;
            mapper.push_back({ent, true});
            return h;
        }

        entt::entity get(EntityHandle handle)
        {
            assert(mapper[handle].second);
            return mapper[handle].first;
        }

        void remove(EntityHandle handle)
        {
            if (handle == k_invalid_handle || handle >= mapper.size())
                return;
            mapper[handle].second = false;
        }

        bool is_valid(EntityHandle handle)
        {
            if (handle == k_invalid_handle || handle >= mapper.size())
                return false;
            return mapper[handle].second;
        }

        bool is_full()
        {
            return mapper.size() >= k_invalid_handle;
        }

    private:
        std::vector<std::pair<entt::entity, bool>> mapper;
    };

    class ECSManager::Impl
    {
    public:
        void t_update()
        {
            // auto view = registry.view<const position, velocity>();

            // // use a callback
            // view.each([](const auto &pos, auto &vel) { /* ... */ });

            // // use an extended callback
            // view.each([](const auto entity, const auto &pos, auto &vel) { /* ... */ });

            // // use a range-for
            // for (auto [entity, pos, vel] : view.each())
            // {
            //     // ...
            // }

            // use forward iterators and get only the components of interest
            // for (auto entity : view)
            // {
            //     auto &vel = view.get<velocity>(entity);
            //     // ...
            // }
        }

        void Init()
        {
            registry = std::make_shared<entt::registry>();
        }

        EntityHandle Create()
        {
            assert(!mapper.is_full());
            auto et = registry->create();
            return mapper.add(et);
        }

        template <typename C>
        void AddComponent(EntityHandle handle, C component)
        {
            assert(mapper.is_valid(handle));
            auto et = mapper.get(handle);
            registry->emplace<C>(et, component);
        }

    private:
        std::shared_ptr<entt::registry> registry = nullptr;
        EntityMapper mapper;
        ECSContext context;
    };

    template <typename C>
    void ECSManager::Add(EntityHandle handle, C component)
    {
        impl->AddComponent(handle, component);
    }

    ECSManager::ECSManager() : impl(std::make_unique<Impl>())
    {
    }

    void ECSManager::Init()
    {
        impl->Init();
    }

    EntityHandle ECSManager::Create()
    {
        return impl->Create();
    }

    void ECSManager::t_init()
    {
        impl->t_init();
    }

    void ECSManager::t_update()
    {
        impl->t_update();
    }

    ECSManager::~ECSManager()
    {
    }
}

#define REG_ECS_SRC
#include "ecs_reg.h"