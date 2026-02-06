#pragma once
#include <GLFW/glfw3.h>
#include <memory>

#include "system/files.h"
#include "utils/planLoader.h"
#include "core/utils.h"
#include "core/controller.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

namespace lmcore
{
    class Window
    {
    public:
        explicit Window(int width, int height) : mWidth(width), mHeight(height) {}
        bool Init();
        void Update();

        bool ShouldClose();
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

        bool ShouldClose();
        void Terminate();

        void PreUpdate();
        void Update();
        void PostUpdate();

        Context GetContext();

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
}