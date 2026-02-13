#include "ecs.h"
#include "core/math.h"

#include "entt/entt.hpp"
#include "core/ecs_impl.h"

namespace lmcore
{
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
        void Init()
        {
            registry = std::make_shared<ECSRegistry>();
        }

        EntityHandle Create()
        {
            assert(!mapper.is_full());
            auto et = registry->reg.create();
            return mapper.add(et);
        }

        template <typename C>
        void AddComponent(EntityHandle handle, C component)
        {
            assert(mapper.is_valid(handle));
            auto et = mapper.get(handle);
            registry->reg.emplace<C>(et, component);
        }

        void AddSystem(std::shared_ptr<ECSSystem> system)
        {
            // TODO
            systems.push_back(system);
        }

        void PreUpdateSystems(uint32_t fbw, uint32_t fbh)
        {
            ECSUpdateContext ctx;
            ctx.frameBufferWidth = fbw;
            ctx.frameBufferHeight = fbh;
            ctx.registry = registry;
            for (auto s : systems)
            {
                s->PreUpdate(ctx);
            }
        }

        void UpdateSystems(double deltaTime)
        {
            ECSUpdateContext ctx;
            ctx.deltaTime = deltaTime;
            ctx.registry = registry;
            for (auto s : systems)
            {
                s->Update(ctx);
            }
        }

        void PostUpdateSystems()
        {
            ECSUpdateContext ctx;
            ctx.registry = registry;
            for (auto s : systems)
            {
                s->PostUpdate(ctx);
            }
        }

    private:
        std::shared_ptr<ECSRegistry> registry = nullptr;
        EntityMapper mapper;
        std::vector<std::shared_ptr<ECSSystem>> systems;
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

    void ECSManager::AddSystem(std::shared_ptr<ECSSystem> system)
    {
        impl->AddSystem(system);
    }

    void ECSManager::PreUpdateSystems(uint32_t fbw, uint32_t fbh)
    {
        impl->PreUpdateSystems(fbw, fbh);
    }

    void ECSManager::UpdateSystems(double deltaTime)
    {
        impl->UpdateSystems(deltaTime);
    }

    void ECSManager::PostUpdateSystems()
    {
        impl->PostUpdateSystems();
    }

    ECSManager::~ECSManager()
    {
    }

    void ECSRenderSystem::PreUpdate(ECSUpdateContext context)
    {
    }

    void ECSRenderSystem::Update(ECSUpdateContext context)
    {
        auto &reg = context.registry->reg;
        auto view = reg.view<RenderComponent, PositionComponent>();

        std::vector<FrameObject> objs;

        for (auto entity : view)
        {
            auto &rc = view.get<RenderComponent>(entity);
            auto &pos = view.get<PositionComponent>(entity);

            FrameObject fo;
            fo.h = rc.handle;
            fo.line = rc.line;
            fo.p = rc.program;
            fo.transform = pos.iso;
            objs.push_back(fo);
        }

        mRenderer->PushFrameObjects(objs);
    }

    void ECSRenderSystem::PostUpdate(ECSUpdateContext context)
    {
    }

    void ECSPhysicalVisualizationSystem::PreUpdate(ECSUpdateContext context)
    {
    }
    void ECSPhysicalVisualizationSystem::Update(ECSUpdateContext context)
    {
        auto &reg = context.registry->reg;
        auto view = reg.view<PhysicalVisualComponent, PositionComponent>();

        std::vector<FrameObject> objs;

        for (auto entity : view)
        {
            auto &rc = view.get<PhysicalVisualComponent>(entity);
            auto &pos = view.get<PositionComponent>(entity);

            FrameObject fo;
            fo.h = rc.handle;
            fo.line = true;
            fo.p = "grid";
            fo.transform = pos.iso;
            objs.push_back(fo);
        }

        mRenderer->PushFrameObjects(objs);
    }
    void ECSPhysicalVisualizationSystem::PostUpdate(ECSUpdateContext context)
    {
    }

    void ECSPhysicSystem::Update(ECSUpdateContext context)
    {
        mSimulator->Update(context.deltaTime);

        auto &reg = context.registry->reg;
        auto view = reg.view<PhysicalComponent, PositionComponent>();

        for (auto entity : view)
        {
            auto &phyc = view.get<PhysicalComponent>(entity);
            auto &pos = view.get<PositionComponent>(entity);

            auto s = mSimulator->GetPhysicalState(phyc.handle);
            pos.iso = s.pose;
        }
    }

    void ECSCameraControlSystem::Update(ECSUpdateContext context)
    {
        auto &reg = context.registry->reg;
        auto view = reg.view<CameraComponent, PositionComponent>();

        // auto keys = ctrlptr->current_key_states;
        auto keys = ctrlptr->temporal_deduct_key_states;
        auto cmst = ctrlptr->current_mouse_states;
        auto lmst = ctrlptr->previous_mouse_states;

        for (auto entity : view)
        {
            auto &cam = view.get<CameraComponent>(entity);
            auto &pos = view.get<PositionComponent>(entity);

            float dt = (float)(context.deltaTime);

            // TODO move to camera/controller
            {
                float moveForward = 0.0f;
                float moveRight = 0.0f;
                float moveUp = 0.0f;

                if (keys.get(EKey::W).is_set(EKeyTemporalState::Down))
                    moveForward += 1.0f;
                if (keys.get(EKey::S).is_set(EKeyTemporalState::Down))
                    moveForward -= 1.0f;
                if (keys.get(EKey::D).is_set(EKeyTemporalState::Down))
                    moveRight -= 1.0f;
                if (keys.get(EKey::A).is_set(EKeyTemporalState::Down))
                    moveRight += 1.0f;
                if (keys.get(EKey::E).is_set(EKeyTemporalState::Down))
                    moveUp += 1.0f;
                if (keys.get(EKey::Q).is_set(EKeyTemporalState::Down))
                    moveUp -= 1.0f;

                if (keys.get(EKey::MouseRight).is_set(EKeyTemporalState::Down))
                {
                    double dx = cmst.lastMouseX - lmst.lastMouseX;
                    double dy = cmst.lastMouseY - lmst.lastMouseY;

                    cam.camera->yaw -= (float)dx * ctrlptr->mouseSensitivity;
                    cam.camera->pitch -= (float)dy * ctrlptr->mouseSensitivity;

                    const float limit = bx::toRad(89.5f);
                    if (cam.camera->pitch > limit)
                        cam.camera->pitch = limit;
                    if (cam.camera->pitch < -limit)
                        cam.camera->pitch = -limit;
                }

                cam.camera->forward = {
                    -std::sin(cam.camera->yaw) * std::cos(cam.camera->pitch),
                    -std::cos(cam.camera->yaw) * std::cos(cam.camera->pitch),
                    std::sin(cam.camera->pitch),
                };
                cam.camera->forward.normalize();

                cam.camera->worldUp = {0.0f, 0.0f, 1.0f};
                cam.camera->right = cam.camera->forward.cross(cam.camera->worldUp);
                cam.camera->right.normalize();
                cam.camera->up = cam.camera->right.cross(cam.camera->forward);

                cam.camera->moveDir = {
                    cam.camera->forward.x() * moveForward + cam.camera->right.x() * moveRight + cam.camera->up.x() * moveUp,
                    cam.camera->forward.y() * moveForward + cam.camera->right.y() * moveRight + cam.camera->up.y() * moveUp,
                    cam.camera->forward.z() * moveForward + cam.camera->right.z() * moveRight + cam.camera->up.z() * moveUp};

                if (moveForward != 0.0f || moveRight != 0.0f || moveUp != 0.0f)
                {
                    cam.camera->moveDir.normalize();
                    cam.camera->position = cam.camera->position + cam.camera->moveDir * (ctrlptr->moveSpeed * dt);
                }

                // pos.iso =
            }
        }
    }
}
#define REG_ECS_SRC
#include "ecs_reg.h"