#pragma once
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/math.h>

#include <GLFW/glfw3.h>

#include "system/files.h"
#include "utils/planLoader.h"
#include "core/utils.h"
#include "core/controller.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

namespace lmcore
{
    // typedef uint32_t RenderProgramHandle;
    typedef uint32_t RenderObjectHandle;

    class Window
    {
    public:
        explicit Window(int width, int height) : mWidth(width), mHeight(height) {}
        bool Init();
        void Update();

        void Terminate();

        int mWidth;
        int mHeight;
        GLFWwindow *mWindow = nullptr;
        Display *x11Display;
        ::Window x11Window;

        std::shared_ptr<Control> mControl;
    };

    struct Camera
    {
        lmcore::Vec3f position = {-10.f, -10.f, 12.5f};
        float yaw = 4.f;
        float pitch = -0.7f;
    };

    class Framework
    {
    public:
        struct Info
        {
            // todo
            uint32_t win_width = 1920;
            uint32_t win_height = 1080;
        };

        struct Context
        {
            std::shared_ptr<Window> win_ptr = nullptr;
            std::shared_ptr<Control> ctrl_ptr = nullptr;
            std::shared_ptr<Camera> cam_ptr = nullptr;
        };

        bool Init(const Info &info);

        void PreUpdate();
        void Update();
        void PostUpdate();

    private:
        Context mctx;
    };

    struct Controller
    {
        bool rotating = false;
        double lastMouseX = 0.0;
        double lastMouseY = 0.0;

        const float moveSpeed = 5.0f;
        const float mouseSensitivity = 0.005f;
    };

    // TODO sort mechanism
    struct FrameObject
    {
        RenderObjectHandle h;
        std::string p;
        bool line = false;
        Iso3f transform;
    };

    class Renderer
    {
    public:
        explicit Renderer(std::shared_ptr<Window> wptr);
        ~Renderer();
        bool Init();
        bool ShouldClose();
        void PreUpdate();
        void Update();
        void PostUpdate();
        void Destroy();
        bool CreateProgram(const std::string &name, const std::string &shadern);
        void PushFrameObject(FrameObject obj);
        void PushFrameObjects(const std::vector<FrameObject> &objs);
        RenderObjectHandle CreateRenderObject(const lmcore::PosColorVertex *const vertices, uint32_t count);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };
}