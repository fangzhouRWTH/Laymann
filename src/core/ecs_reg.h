// template void ECSManager::Add<RenderComponent>(EntityHandle handle, RenderComponent component);

#ifndef REG_ECS_SRC
#include "ecs.cpp"
#endif

#define REG_ECS_COMP(COMP) \
    template void ECSManager::Add<COMP>(EntityHandle handle, COMP component);

namespace lmcore
{
    REG_ECS_COMP(RenderComponent)
    REG_ECS_COMP(PhysicalVisualComponent)
    REG_ECS_COMP(PhysicalComponent)
    REG_ECS_COMP(PositionComponent)
    REG_ECS_COMP(CameraComponent)
}