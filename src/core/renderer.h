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
#include "core/framework.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

namespace lmcore
{
    // typedef uint32_t RenderProgramHandle;
    typedef uint32_t RenderObjectHandle;
    typedef uint32_t RenderTextureHandle;

    // TODO sort mechanism
    struct FrameObject
    {
        RenderObjectHandle h;
        std::string p;
        bool line = false;
        Iso3f transform;
    };

    struct RendererUpdateContext
    {
        uint32_t width = 0u;
        uint32_t height = 0u;

        std::shared_ptr<Camera> cam_ptr;
    };

    class Renderer
    {
    public:
        explicit Renderer(std::shared_ptr<Window> wptr);
        ~Renderer();
        bool Init();

        void SetCamera(std::shared_ptr<Camera> camptr);

        void PreUpdate(const RendererUpdateContext & ctx);
        void Update(const RendererUpdateContext & ctx);
        void PostUpdate(const RendererUpdateContext & ctx);
        void Destroy();
        bool CreateProgram(const std::string &name, const std::string &shadern);
        void PushFrameObject(FrameObject obj);
        void PushFrameObjects(const std::vector<FrameObject> &objs);

        RenderObjectHandle CreateRenderObject(const lmcore::PosColorVertex *const vertices, uint32_t count);
        
        RenderTextureHandle CreateTexture2D(const uint32_t width, const uint32_t height, TextureFormat format, void * data);
        void UpdateTexture2D(const RenderTextureHandle handle, void * data);
    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };
}