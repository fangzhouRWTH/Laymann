#pragma once
#include <cstdint>
#include <vector>
#include "core/renderer.h"
#include "core/simulator.h"

//NOT REALLY AN ECS SYSTEM (TODO, BUT NOT NECESSARY)

namespace lmcore
{
    typedef uint32_t EntityHandle;
    typedef uint32_t ComponentHandle;

    #define k_invalid_handle ComponentHandle(0xFFFFFFFF);

    struct RenderComponent
    {
        RenderObjectHandle handle;
    };

    struct PhysicalComponent
    {
        PhysicalObjectHandle handle;
    };

    struct Entity
    {
        bool dead = false;
        bool active = true;
        ComponentHandle renderComponentHandle = k_invalid_handle;
        ComponentHandle physicalComponentHandle = k_invalid_handle;
    };

    struct EntityPool
    {
        std::vector<Entity> entities;
    };

    class EntityManager
    {
        public:
            EntityHandle RegisterEntity(){}

        private:
            EntityPool mPool;
            std::vector<RenderComponent> mRenderComponentPool;
            std::vector<PhysicalComponent> mPhysicalComponentPool;
    };
}