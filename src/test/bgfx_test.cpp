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
// 在包含 glfw3native 之前定义
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

// -----------------------------
// 简单 Vec3 工具
// -----------------------------
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

// -----------------------------
// 立方体顶点格式
// -----------------------------
struct PosColorVertex
{
    float x, y, z;
    float r, g, b, a;
};

static bgfx::VertexLayout s_PosColorLayout;

static void initVertexLayout()
{
    s_PosColorLayout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Float, true)
        .end();
}

// 定义一个单位立方体（边长 2，中心在原点）的 8 个顶点 + 36 个索引
static PosColorVertex s_cubeVertices[8] =
{
    //  x      y      z       color(ABGR)
    { -1.0f, -1.0f, -1.0f, 0.2, 0.2, 0.2, 1.0 }, // 0
    {  1.0f, -1.0f, -1.0f, 0.8, 0.2, 0.2, 1.0 }, // 1
    {  1.0f,  1.0f, -1.0f, 0.8, 0.2, 0.2, 1.0 }, // 2
    { -1.0f,  1.0f, -1.0f, 0.8, 0.2, 0.2, 1.0 }, // 3
    { -1.0f, -1.0f,  1.0f, 0.8, 0.2, 0.2, 1.0 }, // 4
    {  1.0f, -1.0f,  1.0f, 0.8, 0.2, 0.2, 1.0 }, // 5
    {  1.0f,  1.0f,  1.0f, 0.8, 0.2, 0.2, 1.0 }, // 6
    { -1.0f,  1.0f,  1.0f, 0.8, 0.2, 0.2, 1.0 }, // 7
};

static const uint16_t s_cubeIndices[36] =
{
    // 后面 (-Z)
    0, 1, 2,
    2, 3, 0,
    // 前面 (+Z)
    6, 5, 4,
    4, 7, 6,
    // 左面 (-X)
    7, 4, 0,
    0, 3, 7,
    // 右面 (+X)
    1, 5, 6,
    6, 2, 1,
    // 上面 (+Y)
    3, 2, 6,
    6, 7, 3,
    // 下面 (-Y)
    5, 1, 0,
    0, 4, 5
};

// -----------------------------
// shader 加载（你需要替换路径和文件名）
// -----------------------------
static bgfx::ShaderHandle loadShaderBin(const char* _path)
{
    // TODO: 按你实际的 shader 输出路径修改，比如 "shaders/glsl/vs_cubes.bin"
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
    // TODO: 这里用你实际编好的 shader .bin 路径
    // 比如： "shaders/vs_simple.bin" / "shaders/fs_simple.bin"
    bgfx::ShaderHandle vsh = loadShaderBin("/home/fangzhou/Project/git_repo/Laymann/src/shaders/vs_basic.bin");
    bgfx::ShaderHandle fsh = loadShaderBin("/home/fangzhou/Project/git_repo/Laymann/src/shaders/fs_basic.bin");

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        std::cerr << "Invalid shader handles!" << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program destroyed */);
}

// -----------------------------
// 主程序
// -----------------------------
int main()
{
    // 1. 初始化 GLFW
    if (!glfwInit())
    {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // 不创建 OpenGL 上下文，让 bgfx 来接管
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

    // 2. 获取 X11 Display / Window
    Display* x11Display = glfwGetX11Display();
    ::Window x11Window  = glfwGetX11Window(window);

    // 3. 初始化 bgfx
    bgfx::Init init;
    init.type = bgfx::RendererType::OpenGL;   // 自动选择渲染后端
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

    // 4. 视口 & 背景清屏颜色（草绿色：0xff00ff00，ABGR）
    const bgfx::ViewId kViewId = 0;
    bgfx::setViewClear(kViewId,
                       BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
                       0xFFFFFFFF,    // 草绿色
                       1.0f,
                       0);
    bgfx::setViewRect(kViewId, 0, 0, (uint16_t)width, (uint16_t)height);

    // 5. 初始化顶点格式 + 创建立方体 VB/IB
    initVertexLayout();

    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
        s_PosColorLayout
    );
    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
        bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices))
    );

    // 6. 创建简单着色器 program（你需要提供对应的 .bin）
    bgfx::ProgramHandle program = createSimpleProgram();
    if (!bgfx::isValid(program))
    {
        std::fprintf(stderr, "Failed to create program. Check your shaders.\n");
        bgfx::shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 7. 相机参数
    Vec3 cameraPos{0.0f, 0.0f, -5.0f};  // 初始在 -Z 轴上看向原点
    float yaw   = 0.0f;                 // 左右转
    float pitch = 0.0f;                 // 上下看

    double lastTime = glfwGetTime();

    bool rotating = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    const float moveSpeed = 5.0f;       // 位移速度
    const float mouseSensitivity = 0.005f; // 鼠标灵敏度

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // 8. 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 计算 deltaTime
        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();

        // 处理窗口大小变化
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (fbW != width || fbH != height)
        {
            width = fbW;
            height = fbH;
            bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
            bgfx::setViewRect(kViewId, 0, 0, (uint16_t)width, (uint16_t)height);
        }

        // ---- 键盘控制相机平移 ----
        float moveForward = 0.0f;
        float moveRight   = 0.0f;
        float moveUp      = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveForward += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveForward -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveRight   -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveRight   += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) moveUp      += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) moveUp      -= 1.0f;

        // ---- 鼠标右键控制视角旋转 ----
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

                yaw   += (float)dx * mouseSensitivity;
                pitch -= (float)dy * mouseSensitivity;

                // 限制 pitch 避免翻转
                const float limit = bx::toRad(89.0f);
                if (pitch >  limit) pitch =  limit;
                if (pitch < -limit) pitch = -limit;
            }
        }
        else
        {
            rotating = false;
        }

        // ---- 根据 yaw/pitch 计算前方向 / 右方向 ----
        // 这里约定：yaw=0,pitch=0 时，看向 -Z
        Vec3 forward{
            -std::sin(yaw) * std::cos(pitch),
             std::sin(pitch),
            -std::cos(yaw) * std::cos(pitch)
        };
        forward = normalize(forward);

        Vec3 worldUp{0.0f, 1.0f, 0.0f};
        Vec3 right = normalize(cross(forward, worldUp));
        Vec3 up    = cross(right, forward); // 重新正交化一下

        // ---- 根据输入移动摄像机 ----
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

        // ---- 计算视图投影矩阵 ----
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
        bx::mtxProj(proj, 60.0f, aspect, 0.1f, 100.0f, caps->homogeneousDepth);
        bgfx::setViewTransform(kViewId, view, proj);

        // ---- 模型矩阵：单位矩阵（立方体放在原点）----
        float mtx[16];
        bx::mtxIdentity(mtx);

        // 开始这一帧
        bgfx::touch(kViewId);

        // 设置变换 + VB/IB + 渲染状态
        bgfx::setTransform(mtx);
        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);
        bgfx::setState(
            BGFX_STATE_WRITE_RGB
          | BGFX_STATE_WRITE_A
          | BGFX_STATE_WRITE_Z
          | BGFX_STATE_DEPTH_TEST_LESS
          | BGFX_STATE_CULL_CW
          | BGFX_STATE_MSAA
        );

        // 提交 draw call
        bgfx::submit(kViewId, program);

        // 提交一帧
        bgfx::frame();
    }

    // 9. 清理
    bgfx::destroy(ibh);
    bgfx::destroy(vbh);
    bgfx::destroy(program);

    bgfx::shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
