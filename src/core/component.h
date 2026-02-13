#pragma once

#include "core/math.h"
#include "core/renderer.h"
#include "core/simulator.h"
#include "core/framework.h"

namespace lmcore
{
    struct RenderComponent
    {
        bool isValid = true;
        RenderObjectHandle handle;
        std::string program;
        bool line = false;
    };

    struct PhysicalVisualComponent
    {
        //bool isValid = true;
        RenderObjectHandle handle;
        //std::string program;
        //bool line = false;
    };

    struct FieldVisualComponent
    {
        RenderObjectHandle handle;
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

    struct CameraComponent
    {
        std::shared_ptr<Camera> camera = nullptr;
    };
}