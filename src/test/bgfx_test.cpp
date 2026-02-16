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

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

static lmcore::PosColorVertex gridVertices[204] = {};
static int gridIndices[204] = {};

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

    auto rootpath = lmcore::getExeFolderPath();
    // auto plan_0_path = rootpath + std::string("/../data/plan/l_singleStudio01.json");
    auto plan_0_path = rootpath + std::string("/../data/plan/l_twoBedroomApartment03.json");
    // auto plan_0_path = rootpath + std::string("/../data/plan/room_box.json");
    auto fp_0 = lmcore::load_floor_plan_from_json(plan_0_path);
    std::vector<lmcore::PosColorVertex> fpline_vertices;
    std::vector<int> fpline_indices;
    lmcore::FloorPlanWallMerger merger(fp_0);
    fp_0 = merger.Merge();
    // lmcore::create_vertices_indices_of_room_geometry(fp_0,fpline_vertices,fpline_indices);
    lmcore::create_vertices_indices_from_merger(merger, fpline_vertices);

    //todo
    std::vector<lmcore::PosColorVertex> temp_walls;
    //std::vector<std::vector<lmcore::PosColorVertex>> vs;
    std::vector<lmcore::FrameObject> fos;
    bool has = false;
    for (auto &w : fp_0.walls)
    {
        for (auto &f : w.data.faces)
        {
            for (auto v : f.shape)
            {
                temp_walls.push_back(v);
            }
        }

        for (auto v : w.data.bands)
        {
            temp_walls.push_back(v);
        }

        for (auto c : w.collisions)
        {

            float x = c.bbox.xyz.x();
            float y = c.bbox.xyz.y();
            float z = c.bbox.xyz.z();
            lmcore::PosColorVertex base = {.r = 1.f, .g = 1.f, .b = 0.f};
            auto p0 = base;
            p0.x = -x;
            p0.y = -y;
            p0.z = -z;
            auto p1 = p0;
            p1.x = x;
            auto p2 = p0;
            p2.z = z;
            auto p3 = p1;
            p3.z = z;

            auto _p0 = p0;
            _p0.y = y;
            auto _p1 = p1;
            _p1.y = y;
            auto _p2 = p2;
            _p2.y = y;
            auto _p3 = p3;
            _p3.y = y;

            //vs.emplace_back();
            fos.emplace_back();
            //auto &v = vs.back();
            auto v = std::vector<lmcore::PosColorVertex>();
            auto &f = fos.back();

            v.push_back(p2);
            v.push_back(p3);
            v.push_back(_p2);
            v.push_back(_p3);
            v.push_back(p2);
            v.push_back(_p2);
            v.push_back(p3);
            v.push_back(_p3);

            v.push_back(p0);
            v.push_back(p1);
            v.push_back(_p0);
            v.push_back(_p1);
            v.push_back(p0);
            v.push_back(_p0);
            v.push_back(p1);
            v.push_back(_p1);

            v.push_back(_p0);
            v.push_back(_p2);
            v.push_back(p0);
            v.push_back(p2);
            v.push_back(_p1);
            v.push_back(_p3);
            v.push_back(p1);
            v.push_back(p3);

            auto rh = rd->CreateRenderObject(v.data(), v.size());
            auto ph = phy->RegisterPhysicalObject(c.bbox, c.pose, 0.f);

            lmcore::PhysicalVisualComponent rc = {.handle = rh};
            lmcore::PhysicalComponent pc = {.handle = ph};

            auto et = ecs->Create();
            ecs->Add(et, rc);
            ecs->Add(et, lmcore::PositionComponent{.iso = c.pose});
        }
    }

    auto wall_line_v = rd->CreateRenderObject(fpline_vertices.data(), fpline_vertices.size());
    lmcore::RenderComponent rc_wall_line = {.handle = wall_line_v, .program = "grid", .line = true};
    auto et_line = ecs->Create();
    ecs->Add(et_line, lmcore::PositionComponent{});
    ecs->Add(et_line, rc_wall_line);

    initGridData();
    auto grid_v = rd->CreateRenderObject(gridVertices, 204);
    lmcore::RenderComponent rc_grid = {.handle = grid_v, .program = "grid", .line = true};
    auto et_grid = ecs->Create();
    ecs->Add(et_grid, lmcore::PositionComponent{});
    ecs->Add(et_grid, rc_grid);

    std::vector<lmcore::PosColorVertex> cubev;
    cubev.reserve(36u);
    float cubescale = 0.2f;
    for (auto p : lmcore::geo_default_cube)
    {
        p.x *= (cubescale * 0.5f);
        p.y *= (cubescale * 0.5f);
        p.z *= (cubescale * 0.5f);
        cubev.push_back(p);
    }

    lmcore::Iso3f fiso = lmcore::Iso3f::Identity();
    fiso.translate(lmcore::Vec3f{0.f, 0.f, -1.f});
    auto planh = phy->RegisterPhysicalObject({.xyz = {100.f, 100.f, 1.f}}, fiso, 0.f);
    auto cube = rd->CreateRenderObject(cubev.data(), 36u);

    for (auto &f : fp_0.floors)
    {
        lmcore::Area2D_XY area;
        for (auto &ff : f.data.baseForm)
        {
            temp_walls.push_back(ff);
            area.triangle_points.push_back({ff.x, ff.y, ff.z});
        }
        
        lmcore::DiscreteSpaceField field = lmcore::DiscreteSpaceField::Create(area,0.2f);

        field.GenerateVertices();
        std::vector<lmcore::PosColorVertex> & vs = field.GetVertices();

        auto vh = rd->CreateRenderObject(vs.data(),vs.size());

        lmcore::Iso3f iso = lmcore::Iso3f::Identity();
        //iso.translate(s.world_pos);
        lmcore::RenderComponent rc = {.handle = vh, .program = "basic", .line = false};
        //lmcore::FieldVisualComponent

        auto et_field = ecs->Create();
        ecs->Add(et_field, lmcore::PositionComponent{.iso = iso});
        ecs->Add(et_field, rc);

        // auto &samples = field.GetSamples();
        // for (auto s : samples)
        // {
        //     lmcore::Iso3f iso = lmcore::Iso3f::Identity();
        //     iso.translate(s.world_pos);
        //     float cubescale = 0.05f;
        //     lmcore::RenderComponent rc = {.handle = cube, .program = "basic", .line = false};

        //     auto et_cube = ecs->Create();
        //     ecs->Add(et_cube, lmcore::PositionComponent{.iso = iso});
        //     ecs->Add(et_cube, rc);
        // }
    }

    auto wall_v = rd->CreateRenderObject(temp_walls.data(), temp_walls.size());
    lmcore::RenderComponent rc_wall = {.handle = wall_v, .program = "basic", .line = false};
    auto et_wall = ecs->Create();
    ecs->Add(et_wall, lmcore::PositionComponent{});
    ecs->Add(et_wall, rc_wall);

    uint32_t x = 10u;
    uint32_t y = 10u;
    uint32_t z = 5u;

    float distance = 0.8f;

    // for (auto _x = 0; _x < x; _x++)
    // {
    //     for (auto _y = 0; _y < y; _y++)
    //     {
    //         for (auto _z = 0; _z < z; _z++)
    //         {
    //             lmcore::Iso3f iso = lmcore::Iso3f::Identity();
    //             iso.translate(lmcore::Vec3f{
    //                 (_x - x * 0.5f) * distance,
    //                 (_y - y * 0.5f) * distance,
    //                 (_z - z * 0.5f) * distance + 20.f});
    //             auto phcube = phy->RegisterPhysicalObject({.xyz = {cubescale * 0.5f, cubescale * 0.5f, cubescale * 0.5f}}, iso, 1.f);
    //             lmcore::RenderComponent rc = {.handle = cube, .program = "basic", .line = false};
    //             lmcore::PhysicalComponent pc = {.handle = phcube};

    //             auto et_cube = ecs->Create();
    //             ecs->Add(et_cube, lmcore::PositionComponent{});
    //             ecs->Add(et_cube, rc);
    //             ecs->Add(et_cube, pc);
    //         }
    //     }
    // }

    auto ecctrl = ecs->Create();
    ecs->Add(ecctrl, lmcore::CameraComponent{.camera = framework.GetContext().cam_ptr});
    ecs->Add(ecctrl, lmcore::PositionComponent{});

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