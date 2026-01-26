#pragma once
#include <cstdio>
#include <memory>

#include "core/data.h"

namespace lmcore
{
    typedef uint32_t PhysicalObjectHandle;

    struct PhysicalState
    {
        Iso3f pose;
    };

    class Simulator
    {
    public:
        Simulator();
        ~Simulator();
        void Init();
        void Update(float deltaTime);
        void Destroy();

        PhysicalObjectHandle RegisterPlan(Vec3f normal, Vec3f location);
        PhysicalObjectHandle RegisterPhysicalObject(BBox boundingBox, Iso3f transform, bool isStatic);
        PhysicalState GetPhysicalState(PhysicalObjectHandle handle);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };
}