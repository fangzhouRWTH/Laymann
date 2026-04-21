#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/math.h>

#include <GLFW/glfw3.h>

#include "system/files.h"
#include "utils/planLoader.h"
#include "core/utils.h"
#include "core/renderer.h"
#include "core/simulator.h"
#include "core/geometry.h"
#include "core/ecs.h"
#include "core/engine.h"
#include "core/space_analyse.h"

#include "citygen/noise.h"
#include "citygen/field.h"
#include "citygen/geometry.h"
#include "citygen/road.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

static lmcore::PosColorVertex gridVertices[204] = {};
static int gridIndices[204] = {};

static lmcore::PosColorVertex fieldVertices[6] = {};

static lmcore::PosColorVertex coordVertices[6] = {};

void initGridData()
{
    float r, g, b, a;
    r = .3;
    g = .5;
    b = 0.9;
    a = 1.0;

    for (int i = 0; i < 51; i++)
    {
        float start = -25.f;
        gridVertices[i * 2] = {start + i, -100.f, 0.01, 0.0, 0.0, 1.0, r, g, b, a};
        gridVertices[i * 2 + 1] = {start + i, 100.f, 0.01, 0.0, 0.0, 1.0, r, g, b, a};
    }

    for (int i = 0; i < 51; i++)
    {
        float start = -25.f;
        int index_offset = 102;
        gridVertices[index_offset + i * 2] = {-100.f, start + i, 0.01, 0.0, 0.0, 1.0, r, g, b, a};
        gridVertices[index_offset + i * 2 + 1] = {100.f, start + i, 0.01, 0.0, 0.0, 1.0, r, g, b, a};
    }

    for (int i = 0; i < 204; i++)
    {
        gridIndices[i] = i;
    }
}

void initCoordData()
{
    coordVertices[0] = lmcore::PosColorVertex{5.0, 5.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0};
    coordVertices[1] = lmcore::PosColorVertex{6.0, 5.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0};

    coordVertices[2] = lmcore::PosColorVertex{5.0, 5.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    coordVertices[3] = lmcore::PosColorVertex{5.0, 6.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};

    coordVertices[4] = lmcore::PosColorVertex{5.0, 5.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0};
    coordVertices[5] = lmcore::PosColorVertex{5.0, 5.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0};
}

void initFieldData(float scale = 1.f)
{
    float a = 1.0;
    fieldVertices[0] = lmcore::PosColorVertex{1.f * scale, 1.0f * scale, -0.01,
                                              0.0, 0.0, 1.0,
                                              1.0, 0.0, 0.0, a,
                                              1.0, 1.0};
    fieldVertices[1] = lmcore::PosColorVertex{-1.0f * scale, 1.0f * scale, -0.01,
                                              0.0, 0.0, 1.0,
                                              0.0, 1.0, 0.0, a,
                                              0.0, 1.0};
    fieldVertices[2] = lmcore::PosColorVertex{-1.0f * scale, -1.0f * scale, -0.01,
                                              0.0, 0.0, 1.0,
                                              0.0, 0.0, 1.0, a,
                                              0.0, 0.0};

    fieldVertices[3] = lmcore::PosColorVertex{1.0f * scale, 1.0f * scale, -0.01,
                                              0.0, 0.0, 1.0,
                                              1.0, 0.0, 0.0, a,
                                              1.0, 1.0};
    fieldVertices[4] = lmcore::PosColorVertex{-1.0f * scale, -1.0f * scale, -0.01,
                                              0.0, 0.0, 1.0,
                                              0.0, 0.0, 1.0, a,
                                              0.0, 0.0};
    fieldVertices[5] = lmcore::PosColorVertex{1.0f * scale, -1.0f * scale, -0.01,
                                              0.0, 0.0, 1.0,
                                              0.0, 1.0, 0.0, a,
                                              1.0, 0.0};
}

int main()
{
    lmcore::Framework framework;
    lmcore::Framework::Info finfo;
    finfo.win_width = 1920;
    finfo.win_height = 1080;
    framework.Init(finfo);
    auto fctx = framework.GetContext();
    auto wd = fctx.win_ptr;
    auto rd = std::make_shared<lmcore::Renderer>(wd);
    rd->Init();
    rd->SetCamera(framework.GetContext().cam_ptr);
    auto phy = std::make_shared<lmcore::Simulator>();
    phy->Init();

    std::shared_ptr<lmcore::ECSManager> ecs = std::make_shared<lmcore::ECSManager>();
    ecs->Init();
    std::shared_ptr<lmcore::ECSPhysicSystem> ecs_phy_sys = std::make_shared<lmcore::ECSPhysicSystem>(phy);
    ecs->AddSystem(ecs_phy_sys);
    std::shared_ptr<lmcore::ECSRenderSystem> ecs_rd_sys = std::make_shared<lmcore::ECSRenderSystem>(rd);
    ecs->AddSystem(ecs_rd_sys);
    std::shared_ptr<lmcore::ECSPhysicalVisualizationSystem> ecs_phy_vis_sys = std::make_shared<lmcore::ECSPhysicalVisualizationSystem>(rd);
    ecs->AddSystem(ecs_phy_vis_sys);
    std::shared_ptr<lmcore::ECSCameraControlSystem> ecs_camctrl_sys = std::make_shared<lmcore::ECSCameraControlSystem>(framework.GetContext().ctrl_ptr);
    ecs->AddSystem(ecs_camctrl_sys);

    assert(rd->CreateProgram("grid", "grid"));
    assert(rd->CreateProgram("basic", "basic"));
    assert(rd->CreateProgram("field_vis", "basic_tex"));

    std::vector<uint8_t> test_tex;
    std::vector<float> field_data;
    std::vector<lmcore::PosColorVertex> temp_tex_vert;
    std::vector<lmcore::PosColorVertex> test_tex_mesh;

    PerlinNoise pnoise;
    uint32_t pn_size = 1024u;
    float field_size = 10.f;
    float height_scale = 0.2f;

    int octave = 6;
    float frequency = 1.2f;
    float amplitude = 1.0f;
    float lacunarity = 2.f;
    float persistance = 0.5f;

    double scale = 1.5;
    int roadnetseed = 3;

    uint32_t test_growth_steps = 1024;
    float test_growth_stepsize = 0.25f;

    for (auto i = 0; i < pn_size; i++) // v ?
    {
        for (auto j = 0; j < pn_size; j++) // u ?
        {
            double _x = double(i) / pn_size * scale;
            double _y = double(j) / pn_size * scale;
            // auto v = pnoise.noise(_x,_y);
            auto ov = pnoise.fbm(_x, _y, octave, frequency, amplitude, lacunarity, persistance);
            auto v = (ov + 1.0) / 2.0;
            // uint8_t vuint8 = uint8_t(v * 255.0);
            lmcore::PosColorVertex mv;
            float _u = float(j) / pn_size;
            float _v = float(i) / pn_size;
            mv.x = field_size * (_u - 0.5);
            mv.y = field_size * (_v - 0.5);
            mv.z = ov * height_scale - 0.02;
            mv.u = _u;
            mv.v = _v;
            field_data.push_back(ov);
            // test_tex.push_back(vuint8);
            temp_tex_vert.push_back(mv);
        }
    }

    // lmcore::Field1F field1f(field_data, pn_size, pn_size, field_size, field_size);
    std::shared_ptr<lmcore::Field1F> field1f = std::make_shared<lmcore::Field1F>(field_data, pn_size, pn_size, field_size, field_size, height_scale);
    {
        for (auto i = 0; i < pn_size; i++) // v ?
        {
            for (auto j = 0; j < pn_size; j++) // u ?
            {
                float x = (float(j) / pn_size - 0.5f) * field_size;
                float y = (float(i) / pn_size - 0.5f) * field_size;
                float v = field1f->Sample(x, y);
                auto _v = (v + 1.f) / 2.f;
                uint8_t vuint8 = uint8_t(_v * 32.0) * 4;
                test_tex.push_back(vuint8);
            }
        }
    }

    // std::vector<lmcore::PosColorVertex> segs;
    std::vector<lmcore::PosColorVertex> seg_verts;
    std::vector<lmcore::GVec3f> seg_pos;
    std::vector<lmcore::GVec2f> stripe_pos;

    bool bFlat = false;
    float move_u = 0.f;
    float move_v = 0.f;
    float move_z = field1f->Sample(move_u, move_v);
    lmcore::GVec3f tv = {move_u, move_v, move_z};

    // lmcore::RoadNet rnet(field_size, field_size, roadnetseed);
    lmcore::FieldsArray farray;
    farray.height_field = field1f;
    lmcore::RoadNetOperator rnetop(field_size, field_size, roadnetseed);
    lmcore::DefaultMainRoadPolicy p1(farray);
    p1.Generate(rnetop,1024,0.1f);
    // lmcore::L1Growth gpolicy1(field1f);
    // gpolicy1.Grow(rnet, test_growth_steps, test_growth_stepsize, 3u);
    // lmcore::L2Growth gpolicy2(field1f);
    // gpolicy2.Grow(rnet, 25, test_growth_stepsize, 8u);
    // gpolicy2.Grow(rnet, 15, test_growth_stepsize, 8u);
    // gpolicy2.Grow(rnet, 10, test_growth_stepsize, 64u);
    // lmcore::L3Growth gpolicy3(field1f);
    // gpolicy3.Grow(rnet, 10, test_growth_stepsize, 64u);
    // gpolicy3.Grow(rnet, 10, test_growth_stepsize, 128u);
    // lmcore::L4Growth gpolicy4(field1f);
    // gpolicy4.Grow(rnet, 10, test_growth_stepsize, 128u);
    // // gpolicy4.Grow(rnet,6,test_growth_stepsize,128u);

    uint32_t strSize = stripe_pos.size();

    {
        auto make_stripe = [](lmcore::GVec3f s, lmcore::GVec3f e, std::vector<lmcore::PosColorVertex> &str_verts, int level)
        {
            float dx = -(e.y - s.y);
            float dy = (e.x - s.x);

            float half = (6 - level) * 0.003f;
            float r, g, b;
            switch (level)
            {
            case 0:
                r = 1.0;
                g = 0.3;
                b = 0.0;
                break;
            case 1:
                r = 1.0;
                g = 0.6;
                b = 0.0;
                break;
            case 2:
                r = 1.0;
                g = 0.9;
                b = 0.0;
                break;
            case 3:
                r = 1.0;
                g = 0.9;
                b = 0.3;
                break;
            case 4:
                r = 1.0;
                g = 0.9;
                b = 0.6;
                break;
            default:
                r = 0.0;
                g = 0.2;
                b = 1.0;
                break;
            }

            lmcore::f_normalize_2d_fast(dx, dy);
            dx *= half;
            dy *= half;
            float ndx = -dx;
            float ndy = -dy;

            lmcore::PosColorVertex p0 = {.x = e.x + ndx, .y = e.y + ndy, .z = e.z, .r = 0.2, .g = 0.2, .b = b, .a = 1.f};
            lmcore::PosColorVertex p1 = {.x = e.x + dx, .y = e.y + dy, .z = e.z, .r = 0.2, .g = 0.2, .b = b, .a = 1.f};
            lmcore::PosColorVertex p2 = {.x = s.x + ndx, .y = s.y + ndy, .z = s.z, .r = r, .g = g, .b = b, .a = 1.f};
            lmcore::PosColorVertex p3 = {.x = s.x + dx, .y = s.y + dy, .z = s.z, .r = r, .g = g, .b = b, .a = 1.f};

            str_verts.push_back(p0);
            str_verts.push_back(p1);
            str_verts.push_back(p3);

            str_verts.push_back(p0);
            str_verts.push_back(p3);
            str_verts.push_back(p2);
        };

        std::vector<lmcore::PosColorVertex> str_verts;

        // auto segidx = rnet.segments.getIndicesArray();
        auto &rnet = rnetop.get();
        auto segidx = rnet.segments.getIndicesArray();

        for (auto sidx : segidx)
        {
            auto &s = rnet.segments.get(sidx);
            auto ns = rnet.nodes[s.startNode].pos;
            auto ne = rnet.nodes[s.endNode].pos;
            float hw = (5 - s.level) * 0.006f + 0.010f;
            make_stripe(ns, ne, str_verts, s.level);
        }

        if (str_verts.size() > 0)
        {
            auto stripe = rd->CreateRenderObject(str_verts.data(), str_verts.size());
            lmcore::RenderComponent rc_str = {.handle = stripe, .program = "basic", .line = false};
            auto et_str = ecs->Create();
            ecs->Add(et_str, lmcore::TransformComponent{});
            ecs->Add(et_str, rc_str);
        }
    }

    // if (rnet.segments.size() > 0)
    // {
    //     for (auto s : rnet.segments)
    //     {
    //         auto start = rnet.nodes[s.startNode];
    //         auto end = rnet.nodes[s.endNode];

    //         auto level = s.level;
    //         float mark = level / 4.f;

    //         lmcore::PosColorVertex p0 = {.x = start.pos.x, .y = start.pos.y, .z = start.pos.z, .r = 1.f, .g = mark, .b = 0.f, .a = 1.f};
    //         lmcore::PosColorVertex p1 = {.x = end.pos.x, .y = end.pos.y, .z = end.pos.z, .r = 0.6, .g = 0.6f, .b = 0.8, .a = 1.f};

    //         seg_verts.push_back(p0);
    //         seg_verts.push_back(p1);
    //     }

    //     auto seg_vis = rd->CreateRenderObject(seg_verts.data(), seg_verts.size());
    //     lmcore::RenderComponent rc_seg = {.handle = seg_vis, .program = "grid", .line = true};
    //     auto et_seg = ecs->Create();
    //     ecs->Add(et_seg, lmcore::TransformComponent{});
    //     ecs->Add(et_seg, rc_seg);
    // }

    for (auto i = 0; i < pn_size - 1; i++) // v ?
    {
        for (auto j = 0; j < pn_size - 1; j++) // u ?
        {
            auto v1 = temp_tex_vert[j + i * pn_size];
            auto v2 = temp_tex_vert[j + i * pn_size + 1u];
            auto v3 = temp_tex_vert[j + (i + 1u) * pn_size];
            auto v4 = temp_tex_vert[j + (i + 1u) * pn_size + 1u];

            test_tex_mesh.push_back(v1);
            test_tex_mesh.push_back(v2);
            test_tex_mesh.push_back(v4);
            test_tex_mesh.push_back(v1);
            test_tex_mesh.push_back(v4);
            test_tex_mesh.push_back(v3);
        }
    }

    auto tth = rd->CreateTexture2D(pn_size, pn_size, lmcore::TextureFormat::R8, test_tex.data());

    if (!bFlat)
    {
        auto field_m = rd->CreateRenderObject(test_tex_mesh.data(), test_tex_mesh.size());
        lmcore::RenderComponent rc_field_mesh = {.handle = field_m, .program = "field_vis", .line = false};
        rc_field_mesh.texHandles[0] = tth;
        rc_field_mesh.tex_count++;
        auto et_field_mesh = ecs->Create();
        ecs->Add(et_field_mesh, lmcore::TransformComponent{});
        ecs->Add(et_field_mesh, rc_field_mesh);
    }
    else
    {
        initFieldData(field_size / 2.f);
        auto field_v = rd->CreateRenderObject(fieldVertices, 6);
        lmcore::RenderComponent rc_field = {.handle = field_v, .program = "field_vis", .line = false};
        rc_field.texHandles[0] = tth;
        rc_field.tex_count++;
        auto et_field = ecs->Create();
        ecs->Add(et_field, lmcore::TransformComponent{});
        ecs->Add(et_field, rc_field);
    }

    auto rootpath = lmcore::getExeFolderPath();

    // std::vector<std::vector<lmcore::PosColorVertex>> vs;
    std::vector<lmcore::FrameObject> fos;

    // initGridData();
    // auto grid_v = rd->CreateRenderObject(gridVertices, 204);
    // lmcore::RenderComponent rc_grid = {.handle = grid_v, .program = "grid", .line = true};
    // auto et_grid = ecs->Create();
    // ecs->Add(et_grid, lmcore::TransformComponent{});
    // ecs->Add(et_grid, rc_grid);

    initCoordData();
    auto coord_v = rd->CreateRenderObject(coordVertices, 6);
    lmcore::RenderComponent rc_coord = {.handle = coord_v, .program = "grid", .line = true};
    auto et_coord = ecs->Create();
    ecs->Add(et_coord, lmcore::TransformComponent{});
    ecs->Add(et_coord, rc_coord);

    auto ecctrl = ecs->Create();
    ecs->Add(ecctrl, lmcore::CameraComponent{.camera = framework.GetContext().cam_ptr});
    ecs->Add(ecctrl, lmcore::TransformComponent{});

    while (!framework.ShouldClose())
    {
        uint32_t wd = 0u;
        uint32_t ht = 0u;
        framework.PreUpdate();
        framework.GetFrameSize(wd, ht);
        float dt = framework.GetDeltaTime();
        lmcore::RendererUpdateContext ctx;
        ctx.width = wd;
        ctx.height = ht;
        rd->PreUpdate(ctx);
        ecs->PreUpdateSystems(wd, ht);

        framework.Update();
        ecs->UpdateSystems(dt);
        rd->Update({});

        framework.PostUpdate();
        ecs->PostUpdateSystems();
        rd->PostUpdate({});
    }

    rd->Destroy();
    phy->Destroy();
    framework.Terminate();
}