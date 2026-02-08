// template void ECSManager::Add<RenderComponent>(EntityHandle handle, RenderComponent component);

#ifndef REG_ECS_SRC
#include "ecs.cpp"
#endif

#define REG_ECS_COMP(COMP) \
    template void ECSManager::Add<COMP>(EntityHandle handle, COMP component);

namespace lmcore
{
    REG_ECS_COMP(RenderComponent)
    REG_ECS_COMP(PhysicalComponent)
    REG_ECS_COMP(PositionComponent)
    REG_ECS_COMP(CameraComponent)

    // template void ECSManager::Add<RenderComponent>(EntityHandle handle, RenderComponent component);
    // template void ECSManager::Add<PhysicalComponent>(EntityHandle handle, PhysicalComponent component);
    // template void ECSManager::Add<PositionComponent>(EntityHandle handle, PositionComponent component);
    // template void ECSManager::Add<CameraComponent>(EntityHandle handle, CameraComponent component);
}