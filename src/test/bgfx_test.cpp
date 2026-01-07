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

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

struct Vec3
{
    float x, y, z;
};

static Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

static Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

static Vec3 operator*(const Vec3& v, float s)
{
    return {v.x * s, v.y * s, v.z * s};
}

static Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static Vec3 normalize(const Vec3& v)
{
    float len2 = v.x*v.x + v.y*v.y + v.z*v.z;
    if (len2 <= 0.0f) return {0.0f, 0.0f, 0.0f};
    float invLen = 1.0f / std::sqrt(len2);
    return {v.x * invLen, v.y * invLen, v.z * invLen};
}

static bgfx::VertexLayout s_PosColorLayout;
//static bgfx::VertexLayout s_LineLayout;

// struct LineVertex
// {
//     float x, y, z;
//     float r,g,b,a;
// };

static void initVertexLayout()
{
    s_PosColorLayout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float, true)
        .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Float, true)
        .end();

    // s_LineLayout
    //     .begin()
    //     .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
    //     .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Float, true) // normalized
    //     .end();
}

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
        gridVertices[i*2] = {start+i,-100.f,0.0, 0.0,0.0,1.0, r,g,b,a};
        gridVertices[i*2+1] = {start+i,100.f,0.0, 0.0,0.0,1.0, r,g,b,a};
    }

    for(int i = 0; i < 51; i++)
    {
        float start = -25.f;
        int index_offset = 102; 
        gridVertices[index_offset + i*2] = {-100.f, start+i,0.0, 0.0,0.0,1.0, r,g,b,a};
        gridVertices[index_offset + i*2 + 1] = {100.f, start+i,0.0, 0.0,0.0,1.0, r,g,b,a};
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

static bgfx::ShaderHandle loadShaderBin(const char* _path)
{
    std::string path = _path;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    const bgfx::Memory* mem = bgfx::alloc(uint32_t(size + 1));
    if (!file.read((char*)mem->data, size))
    {
        std::cerr << "Failed to read shader file: " << path << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    mem->data[size] = '\0';

    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, path.c_str(), (uint16_t)path.size());

    return handle;
}

static bgfx::ProgramHandle createSimpleProgram()
{
    auto shaderpath = lmv::getShaderPath();
    bgfx::ShaderHandle vsh = loadShaderBin((shaderpath + "/vs_basic.bin").c_str());
    bgfx::ShaderHandle fsh = loadShaderBin((shaderpath + "/fs_basic.bin").c_str());

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        std::cerr << "Invalid shader handles!" << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program destroyed */);
}

static bgfx::ProgramHandle createGridProgram()
{
    auto shaderpath = lmv::getShaderPath();
    bgfx::ShaderHandle vsh = loadShaderBin((shaderpath + "/vs_grid.bin").c_str());
    bgfx::ShaderHandle fsh = loadShaderBin((shaderpath + "/fs_grid.bin").c_str());

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        std::cerr << "Invalid shader handles!" << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program destroyed */);
}

int main()
{
    auto rootpath = lmv::getExeFolderPath();
    auto plan_0_path = rootpath + std::string("/data/plan/l_singleStudio01.json");
    auto fp_0 = lmv::load_floor_plan_from_json(plan_0_path);

    if (!glfwInit())
    {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    int width  = 1920;
    int height = 1080;

    GLFWwindow* window = glfwCreateWindow(width, height, "bgfx minimal (Linux + camera + cube)", nullptr, nullptr);
    if (!window)
    {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    Display* x11Display = glfwGetX11Display();
    ::Window x11Window  = glfwGetX11Window(window);

    bgfx::Init init;
    init.type = bgfx::RendererType::OpenGL;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.platformData.ndt = x11Display;
    init.platformData.nwh = (void*)(uintptr_t)x11Window;
    init.platformData.context = nullptr;
    init.platformData.backBuffer = nullptr;
    init.platformData.backBufferDS = nullptr;

    init.resolution.width  = (uint32_t)width;
    init.resolution.height = (uint32_t)height;
    init.resolution.reset  = BGFX_RESET_VSYNC;

    if (!bgfx::init(init))
    {
        std::fprintf(stderr, "Failed to init bgfx\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    const bgfx::ViewId kViewId = 0;
    bgfx::setViewClear(kViewId,
                       BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
                       0x00000000,
                       1.0f,
                       0);
    bgfx::setViewRect(kViewId, 0, 0, (uint16_t)width, (uint16_t)height);

    initVertexLayout();
    initGridData();

    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(cubeVertices, sizeof(cubeVertices)),
        s_PosColorLayout
    );
    // bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
    //     bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices))
    // );
    bgfx::VertexBufferHandle vbh_grid = bgfx::createVertexBuffer(bgfx::makeRef(gridVertices, sizeof(gridVertices)),s_PosColorLayout);
    bgfx::IndexBufferHandle ibh_grid = bgfx::createIndexBuffer(bgfx::makeRef(gridIndices, sizeof(gridIndices)));

    bgfx::ProgramHandle program = createSimpleProgram();
    if (!bgfx::isValid(program))
    {
        std::fprintf(stderr, "Failed to create program. Check your shaders.\n");
        bgfx::shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    bgfx::ProgramHandle program_grid = createGridProgram();
    if (!bgfx::isValid(program_grid))
    {
        std::fprintf(stderr, "Failed to create program. Check your shaders.\n");
        bgfx::shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    bgfx::UniformHandle u_camera = bgfx::createUniform("u_camera", bgfx::UniformType::Vec4);

    Vec3 cameraPos{0.0f, -5.0f, 0.0f}; 
    float yaw   = 3.1415926f;                
    float pitch = 0.0f;                

    double lastTime = glfwGetTime();

    bool rotating = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    const float moveSpeed = 5.0f; 
    const float mouseSensitivity = 0.005f; 

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (fbW != width || fbH != height)
        {
            width = fbW;
            height = fbH;
            bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
            bgfx::setViewRect(kViewId, 0, 0, (uint16_t)width, (uint16_t)height);
        }

        float moveForward = 0.0f;
        float moveRight   = 0.0f;
        float moveUp      = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveForward += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveForward -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveRight   -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveRight   += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) moveUp      += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) moveUp      -= 1.0f;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);

            if (!rotating)
            {
                rotating = true;
                lastMouseX = mx;
                lastMouseY = my;
            }
            else
            {
                double dx = mx - lastMouseX;
                double dy = my - lastMouseY;
                lastMouseX = mx;
                lastMouseY = my;

                yaw   -= (float)dx * mouseSensitivity;
                pitch -= (float)dy * mouseSensitivity;

                const float limit = bx::toRad(89.0f);
                if (pitch >  limit) pitch =  limit;
                if (pitch < -limit) pitch = -limit;
            }
        }
        else
        {
            rotating = false;
        }

        Vec3 forward{
            -std::sin(yaw) * std::cos(pitch),
            -std::cos(yaw) * std::cos(pitch),
            std::sin(pitch),
        };
        forward = normalize(forward);

        Vec3 worldUp{0.0f, 0.0f, 1.0f};
        Vec3 right = normalize(cross(forward, worldUp));
        Vec3 up    = cross(right, forward);

        Vec3 moveDir{
            forward.x * moveForward + right.x * moveRight + up.x * moveUp,
            forward.y * moveForward + right.y * moveRight + up.y * moveUp,
            forward.z * moveForward + right.z * moveRight + up.z * moveUp
        };

        if (moveForward != 0.0f || moveRight != 0.0f || moveUp != 0.0f)
        {
            moveDir = normalize(moveDir);
            cameraPos = cameraPos + moveDir * (moveSpeed * dt);
        }

        float view[16];
        float proj[16];

        bx::Vec3 eye = { cameraPos.x, cameraPos.y, cameraPos.z };
        bx::Vec3 at  = { cameraPos.x + forward.x,
                         cameraPos.y + forward.y,
                         cameraPos.z + forward.z };
        bx::Vec3 upArr = { up.x, up.y, up.z };

        bx::mtxLookAt(view, eye, at, upArr);

        float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
        const bgfx::Caps* caps = bgfx::getCaps();
        float nearp = 0.1f;
        float farp = 100.f;
        float nf[4] = {nearp, farp, 0, 0};
        bx::mtxProj(proj, 60.0f, aspect, nearp, farp, caps->homogeneousDepth);
        bgfx::setViewTransform(kViewId, view, proj);

        float mtx[16];
        bx::mtxIdentity(mtx);

        bgfx::touch(kViewId);

        bgfx::setTransform(mtx);

        bgfx::setVertexBuffer(0, vbh_grid);
        //bgfx::setIndexBuffer(ibh_grid);
        bgfx::setUniform(u_camera, nf);
        bgfx::setState(
            BGFX_STATE_WRITE_RGB
          | BGFX_STATE_WRITE_A
          | BGFX_STATE_WRITE_Z
          | BGFX_STATE_DEPTH_TEST_LESS
          | BGFX_STATE_CULL_CW
          | BGFX_STATE_MSAA
          | BGFX_STATE_PT_LINES
        );
        bgfx::submit(kViewId, program_grid);

        bgfx::setVertexBuffer(0, vbh);
        //bgfx::setIndexBuffer(ibh);
        bgfx::setUniform(u_camera, nf);
        bgfx::setState(
            BGFX_STATE_WRITE_RGB
          | BGFX_STATE_WRITE_A
          | BGFX_STATE_WRITE_Z
          | BGFX_STATE_DEPTH_TEST_LESS
          | BGFX_STATE_CULL_CW
          | BGFX_STATE_MSAA
        );
        bgfx::submit(kViewId, program);

        bgfx::frame();
    }

    //bgfx::destroy(ibh);
    bgfx::destroy(ibh_grid);
    bgfx::destroy(u_camera);
    bgfx::destroy(vbh);
    bgfx::destroy(vbh_grid);
    bgfx::destroy(program);
    bgfx::destroy(program_grid);

    bgfx::shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
