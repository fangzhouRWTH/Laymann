#include "core/space_analyse.h"

namespace lmcore
{
    // std::array<PosColorVertex,18u> DiscreteSpaceField::s_vis_verts = {

    // };
    std::array<Vec3f,6u> DiscreteSpaceField::s_vis_verts = {
        Vec3f{1.0,0.0,0.0},
        Vec3f{0.0,1.0,0.0},
        Vec3f{-1.0,0.0,0.0},

        Vec3f{-1.0,0.0,0.0},
        Vec3f{0.0,-1.0,0.0},
        Vec3f{1.0,0.0,0.0},
    };
}