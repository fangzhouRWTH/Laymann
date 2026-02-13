#include "core/ecs_systems.h"
#include "core/component.h"
#include "core/ecs_impl.h"

namespace lmcore
{
    void ECSFieldAnalysisVisualSystem::PreUpdate(ECSUpdateContext context)
    {
    }

    void ECSFieldAnalysisVisualSystem::Update(ECSUpdateContext context)
    {
        auto &reg = context.registry->reg;
        auto view = reg.view<FieldVisualComponent, PositionComponent>();

        std::vector<FrameObject> objs;

        for (auto entity : view)
        {
            auto &rc = view.get<FieldVisualComponent>(entity);
            auto &pos = view.get<PositionComponent>(entity);

            FrameObject fo;
            fo.h = rc.handle;
            fo.line = false;
            fo.p = "field";
            fo.transform = pos.iso;
            objs.push_back(fo);
        }

        mRenderer->PushFrameObjects(objs);
    }
    void ECSFieldAnalysisVisualSystem::PostUpdate(ECSUpdateContext context)
    {
    }
}