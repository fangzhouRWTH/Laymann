#pragma once

#include <stdint.h>

#include "core/ecs.h"

namespace lmcore
{
    class ECSFieldAnalysisVisualSystem final : public ECSSystem
    {
    public:
        ECSFieldAnalysisVisualSystem(std::shared_ptr<Renderer> renderer) : mRenderer(renderer) {};

        virtual void PreUpdate(ECSUpdateContext context);
        virtual void Update(ECSUpdateContext context);
        virtual void PostUpdate(ECSUpdateContext context);

    private:
        std::shared_ptr<Renderer> mRenderer;
    };
}