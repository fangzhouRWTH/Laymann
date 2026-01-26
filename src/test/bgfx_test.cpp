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
    // btDefaultCollisionConfiguration * collisionConfiguration = new btDefaultCollisionConfiguration();
    auto wd = std::make_shared<lmcore::Window>(1920, 1080);
    wd->Init();
    auto rd = std::make_shared<lmcore::Renderer>(wd);
    rd->Init();
    auto phy = std::make_shared<lmcore::Simulator>();

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

    std::vector<lmcore::PosColorVertex> temp_walls;
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
    }

    for (auto &f : fp_0.floors)
    {
        for (auto &ff : f.data.baseForm)
        {
            temp_walls.push_back(ff);
        }
    }

    // lmcore::Mat4f idmtx = lmcore::Mat4f::Identity();
    lmcore::Iso3f idmtx = lmcore::Iso3f::Identity();
    auto wall_v = rd->CreateRenderObject(temp_walls.data(), temp_walls.size());
    lmcore::FrameObject wallObj;
    wallObj.h = wall_v;
    wallObj.p = "basic";
    wallObj.line = false;
    wallObj.transform = idmtx;

    auto wall_line_v = rd->CreateRenderObject(fpline_vertices.data(), fpline_vertices.size());
    lmcore::FrameObject walllineObj;
    walllineObj.h = wall_line_v;
    walllineObj.p = "grid";
    walllineObj.line = true;
    walllineObj.transform = idmtx;

    initGridData();
    auto grid_v = rd->CreateRenderObject(gridVertices, 204);
    lmcore::FrameObject gridObj;
    gridObj.h = grid_v;
    gridObj.p = "grid";
    gridObj.line = true;
    gridObj.transform = idmtx;

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

    phy->Init();
    phy->RegisterPlan({0.f, 0.f, 1.f}, {0.f, 0.f, 0.f});

    auto cube = rd->CreateRenderObject(cubev.data(), 36u);

    std::vector<lmcore::FrameObject> fObjs;
    std::vector<lmcore::PhysicalObjectHandle> pHandles;

    uint32_t x = 10u;
    uint32_t y = 10u;
    uint32_t z = 5u;

    float distance = 0.6f;

    for (auto _x = 0; _x < x; _x++)
    {
        for (auto _y = 0; _y < y; _y++)
        {
            for (auto _z = 0; _z < z; z++)
            {
                lmcore::Iso3f iso = lmcore::Iso3f::Identity();
                iso.translate(lmcore::Vec3f{
                        (_x - x * 0.5f) * distance,
                        (_y - y * 0.5f) * distance, 
                        (_z - z * 0.5f) * distance});
                lmcore::FrameObject cubeObj;
                cubeObj.h = cube;
                cubeObj.p = "basic";
                cubeObj.line = false;
                cubeObj.transform = iso;
                auto phcube = phy->RegisterPhysicalObject({.xyz = {cubescale * 0.5f, cubescale * 0.5f, cubescale * 0.5f}}, iso, false);
                fObjs.push_back(cubeObj);
                pHandles.push_back(phcube);
            }
        }
    }

    lmcore::Iso3f iso = lmcore::Iso3f::Identity();
    iso.translate(lmcore::Vec3f{0.f, 0.f, 10.f});
    lmcore::FrameObject cubeObj;
    cubeObj.h = cube;
    cubeObj.p = "basic";
    cubeObj.line = false;
    cubeObj.transform = iso;
    auto phcube = phy->RegisterPhysicalObject({.xyz = {cubescale * 0.5f, cubescale * 0.5f, cubescale * 0.5f}}, iso, false);

    while (!rd->ShouldClose())
    {
        phy->Update(0.16f);

        rd->PushFrameObject(gridObj);
        rd->PushFrameObject(walllineObj);
        rd->PushFrameObject(wallObj);

        auto cubestate = phy->GetPhysicalState(phcube);
        cubeObj.transform = cubestate.pose;
        rd->PushFrameObject(cubeObj);

        rd->PreUpdate();

        rd->Update();
        rd->PostUpdate();
    }

    rd->Destroy();
    wd->Terminate();
    phy->Destroy();
}