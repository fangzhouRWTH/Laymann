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

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

namespace lmcore
{
//typedef uint32_t RenderProgramHandle;
typedef uint32_t RenderObjectHandle;

class Window
{
    public:
        explicit Window(int width, int height):mWidth(width),mHeight(height){}
        bool Init()
        {
            if (!glfwInit())
            {
                std::fprintf(stderr, "Failed to initialize GLFW\n");
                return false;
            }
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            mWindow = glfwCreateWindow(mWidth,mHeight,"Laymann v 0 0 1", nullptr, nullptr);
            if (mWindow == nullptr)
            {
                std::fprintf(stderr, "Failed to create GLFW window\n");
                glfwTerminate();
                return false;
            }

            glfwMakeContextCurrent(mWindow);
            glfwSwapInterval(0);
            //todo specify platform
            x11Display = glfwGetX11Display();
            x11Window  = glfwGetX11Window(mWindow);
            return true;
        }

        void Terminate()
        {
            glfwDestroyWindow(mWindow);
            glfwTerminate();
        }

        int mWidth;
        int mHeight;
        GLFWwindow* mWindow = nullptr;
        Display* x11Display;
        ::Window x11Window;
};

struct Camera
{
    lmcore::Vec3f position = {-10.f,-10.f,12.5f};
    float yaw = 4.f;
    float pitch = -0.7f;


};

struct Controller
{
    bool rotating = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    const float moveSpeed = 5.0f; 
    const float mouseSensitivity = 0.005f; 
};

//TODO sort mechanism
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
        bool CreateProgram(const std::string & name, const std::string & shadern);
        void PushFrameObject(FrameObject obj);
        void PushFrameObjects(const std::vector<FrameObject> & objs);
        RenderObjectHandle CreateRenderObject(const lmcore::PosColorVertex* const vertices, uint32_t count);
    
    private:
        class Impl;
        std::unique_ptr<Impl> impl;

};
}