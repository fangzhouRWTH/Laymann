#pragma once

#include "core/math.h"
#include "core/renderer.h"
#include "core/simulator.h"

namespace lmcore
{
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

    struct PositionComponent
    {
        Iso3f iso = Iso3f::Identity();
    };

}