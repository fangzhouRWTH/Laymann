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

#include <btBulletDynamicsCommon.h>

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

static lmcore::PosColorVertex gridVertices[204] = {};
static int gridIndices[204] = {};

void initGridData()
{
    float r,g,b,a;
    r = .3;
    g = .5;
    b = 0.9;
    a = 1.0;

    for(int i = 0; i < 51; i++)
    {
        float start = -25.f;
        gridVertices[i*2] = {start+i,-100.f,0.01, 0.0,0.0,1.0, r,g,b,a};
        gridVertices[i*2+1] = {start+i,100.f,0.01, 0.0,0.0,1.0, r,g,b,a};
    }

    for(int i = 0; i < 51; i++)
    {
        float start = -25.f;
        int index_offset = 102; 
        gridVertices[index_offset + i*2] = {-100.f, start+i,0.01, 0.0,0.0,1.0, r,g,b,a};
        gridVertices[index_offset + i*2 + 1] = {100.f, start+i,0.01, 0.0,0.0,1.0, r,g,b,a};
    }

    for(int i = 0; i < 204; i++)
    {
        gridIndices[i] = i;
    }
}

static lmcore::PosColorVertex cubeVertices[36] = {
    // +X face (right)
    {1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},  // red
    {1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},
    {1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},

    {1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},
    {1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},
    {1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f},

    // -X face (left)
   {-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 1.0f},  // cyan
   {-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 1.0f},
   {-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 1.0f},

   {-1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 1.0f},
   {-1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 1.0f},
   {-1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 1.0f},

    // +Y face (top)
   {-1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f},  // green
    {1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f},
    {1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f},

    {1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f},
   {-1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f},
   {-1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f},

    // -Y face (bottom)
   {-1.0f, -1.0f,  1.0f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f, 1.0f},  // magenta
    {1.0f, -1.0f,  1.0f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f, 1.0f},
    {1.0f, -1.0f, -1.0f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f, 1.0f},

    {1.0f, -1.0f, -1.0f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f, 1.0f},
   {-1.0f, -1.0f, -1.0f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f, 1.0f},
   {-1.0f, -1.0f,  1.0f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f, 1.0f},

    // +Z face (front)  // blue
    {1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f},
    {-1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f},
    {1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f},

   {-1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f},
   {1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f},
   {-1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f},

    // -Z face (back) // yellow
   {-1.0f, -1.0f, -1.0f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f, 0.0f},
   {1.0f, -1.0f, -1.0f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f, 0.0f}, 
   {-1.0f,  1.0f, -1.0f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f, 0.0f},

    {1.0f,  1.0f, -1.0f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f, 0.0f},
    {-1.0f,  1.0f, -1.0f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f, 0.0f},
    {1.0f, -1.0f, -1.0f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f, 0.0f}
};

int main()
{
    //btDefaultCollisionConfiguration * collisionConfiguration = new btDefaultCollisionConfiguration();
    auto wd = std::make_shared<lmcore::Window>(1920,1080);
    wd->Init();
    auto rd = std::make_shared<lmcore::Renderer>(wd);
    rd->Init();

    assert(rd->CreateProgram("grid","grid"));
    assert(rd->CreateProgram("basic","basic"));

    initGridData();
    auto grid_v = rd->CreateRenderObject(gridVertices, 204);
    lmcore::FrameObject gridObj;
    gridObj.h = grid_v;
    gridObj.p = "grid";
    gridObj.line = true;

    auto rootpath = lmcore::getExeFolderPath();
    //auto plan_0_path = rootpath + std::string("/../data/plan/l_singleStudio01.json");
    auto plan_0_path = rootpath + std::string("/../data/plan/l_twoBedroomApartment03.json");
    //auto plan_0_path = rootpath + std::string("/../data/plan/room_box.json");
    auto fp_0 = lmcore::load_floor_plan_from_json(plan_0_path);
    std::vector<lmcore::PosColorVertex> fpline_vertices;
    std::vector<int> fpline_indices;
    lmcore::FloorPlanWallMerger merger(fp_0);
    fp_0 = merger.Merge();
    //lmcore::create_vertices_indices_of_room_geometry(fp_0,fpline_vertices,fpline_indices);
    lmcore::create_vertices_indices_from_merger(merger,fpline_vertices);

    std::vector<lmcore::PosColorVertex> temp_walls;
    for(auto & w : fp_0.walls)
    {
        for(auto & f : w.data.faces)
        {
            for(auto v : f.shape)
            {
                temp_walls.push_back(v);
            }
        }

        for(auto v : w.data.bands)
        {
            temp_walls.push_back(v);
        }
    }

    for(auto & f : fp_0.floors)
    {
        for(auto & ff : f.data.baseForm)
        {
            temp_walls.push_back(ff);
        }
    }

    auto wall_v = rd->CreateRenderObject(temp_walls.data(),temp_walls.size());
    lmcore::FrameObject wallObj;
    wallObj.h = wall_v;
    wallObj.p = "basic"; 
    wallObj.line = false;

    auto wall_line_v = rd->CreateRenderObject(fpline_vertices.data(),fpline_vertices.size());
    lmcore::FrameObject walllineObj;
    walllineObj.h = wall_line_v;
    walllineObj.p = "grid"; 
    walllineObj.line = true;

    while(!rd->ShouldClose())
    {
        rd->PushFrameObject(gridObj);
        rd->PushFrameObject(walllineObj);
        rd->PushFrameObject(wallObj);

        rd->PreUpdate();
        rd->Update();
        rd->PostUpdate();
    }

    rd->Destroy();
    wd->Terminate();
}