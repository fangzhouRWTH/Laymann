#include "core/renderer.h"
#include "renderer.h"

namespace lmcore
{

    static bgfx::ShaderHandle loadShaderBin(const char *_path)
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

        const bgfx::Memory *mem = bgfx::alloc(uint32_t(size + 1));
        if (!file.read((char *)mem->data, size))
        {
            std::cerr << "Failed to read shader file: " << path << std::endl;
            return BGFX_INVALID_HANDLE;
        }

        mem->data[size] = '\0';

        bgfx::ShaderHandle handle = bgfx::createShader(mem);
        bgfx::setName(handle, path.c_str(), (uint16_t)path.size());

        return handle;
    }

    struct RProgram
    {
        bgfx::ShaderHandle vsh;
        bgfx::ShaderHandle fsh;
        bgfx::ProgramHandle pgh;
    };

    struct RObject
    {
        bool destroyed = false;
        bgfx::VertexBufferHandle vbh;
        bgfx::IndirectBufferHandle ibh;
        bool indexDraw = false;
    };

    struct RenderObjectContainer
    {
        RenderObjectHandle add(RObject obj)
        {
            robjs.push_back(obj);
            return RenderObjectHandle(robjs.size() - 1);
        }

        void destroy(RenderObjectHandle handle)
        {
            assert(handle < robjs.size());
            auto &o = robjs[handle];
            if (o.destroyed)
                return;
            bgfx::destroy(o.vbh);
            if (o.indexDraw)
                bgfx::destroy(o.ibh);
            o.destroyed = true;
        }

        void destroy()
        {
            for (auto &o : robjs)
            {
                if (o.destroyed)
                    continue;
                bgfx::destroy(o.vbh);
                if (o.indexDraw)
                    bgfx::destroy(o.ibh);
                o.destroyed = true;
            }
        }

        std::vector<RObject> robjs;
    };

    // TODO
    struct FrameObjectContainer
    {
        std::vector<FrameObject> objs;
    };

    // TODO
    struct UploadBufferCache
    {
        uint32_t current_offset = 0;
        std::vector<byte> buffer;

        void* push(void * data, uint32_t size)
        {
            uint32_t csize = buffer.size();
            uint32_t new_offset = current_offset + size;
            //if (new_offset > )
        }
    };

    class Renderer::Impl
    {
    public:
        Impl(std::shared_ptr<Window> wptr) : mWindowPtr(wptr)
        {
        }

        bool Init()
        {
            // todo use safe init
            bgfx::Init init;
            init.type = bgfx::RendererType::OpenGL;
            init.vendorId = BGFX_PCI_ID_NONE;
            init.platformData.ndt = mWindowPtr->x11Display;
            init.platformData.nwh = (void *)(uintptr_t)(mWindowPtr->x11Window);
            init.platformData.context = nullptr;
            init.platformData.backBuffer = nullptr;
            init.platformData.backBufferDS = nullptr;

            init.resolution.width = (uint32_t)mWindowPtr->mWidth;
            init.resolution.height = (uint32_t)mWindowPtr->mHeight;
            mFrameBufferWidth = (uint32_t)mWindowPtr->mWidth;
            mFrameBufferHeight = (uint32_t)mWindowPtr->mHeight;
            init.resolution.reset = BGFX_RESET_VSYNC;

            if (!bgfx::init(init))
                return false;
            bgfx::setViewClear(mViewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.f, 0);
            bgfx::setViewRect(mViewId, 0, 0, (uint16_t)mWindowPtr->mWidth, (uint16_t)mWindowPtr->mHeight);

            mPosColorLayout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float, true)
                .end();

            // TODO move out timer
            mLastTime = glfwGetTime();

            initDefaultUniforms();

            return true;
        }

        void SetCamera(std::shared_ptr<Camera> camptr)
        {
            mCamera = camptr;
        }

        bool CreateProgram(const std::string &name, const std::string &shadern)
        {
            auto shaderpath = getShaderPath();
            bgfx::ShaderHandle vsh = loadShaderBin((shaderpath + "/vs_" + shadern + ".bin").c_str());
            bgfx::ShaderHandle fsh = loadShaderBin((shaderpath + "/fs_" + shadern + ".bin").c_str());
            if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
            {
                std::cerr << "Invalid shader handles!" << std::endl;
                return false;
            }

            if (mPrograms.find("name") != mPrograms.end())
            {
                std::cerr << "Program name is used already!" << std::endl;
                return false;
            }

            auto pgh = bgfx::createProgram(vsh, fsh, true);

            mPrograms.insert({name, RProgram{.vsh = vsh, .fsh = fsh, .pgh = pgh}});
            return true;
        }

        RenderObjectHandle CreateRenderObject(const lmcore::PosColorVertex *const vertices, uint32_t count)
        {
            auto vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices, count * sizeof(lmcore::PosColorVertex)), mPosColorLayout);
            RObject obj{.vbh = vbh};
            return mRenderObjects.add(obj);
        }

        void PushFrameObject(FrameObject obj)
        {
            mFrameObjects.objs.push_back(obj);
        }

        void PushFrameObjects(const std::vector<FrameObject> &objs)
        {
            mFrameObjects.objs.insert(mFrameObjects.objs.end(), objs.begin(), objs.end());
        }

        template <typename T>
        void CreateUniform(const std::string &name)
        {
            assert(mUniforms.find(name) == mUniforms.end());
            bgfx::UniformHandle handle;
            if constexpr (std::is_same_v<T, lmcore::Vec4f>)
            {
                handle = bgfx::createUniform("u_camera", bgfx::UniformType::Vec4);
            }
            else
            {
            }

            mUniforms.insert({name, handle});
        }

        void UpdateUniform(const std::string &name, void *data, uint32_t size)
        {
            auto hi = mUniforms.find(name);

            assert(hi != mUniforms.end());
            auto h = hi->second;
            assert((h.idx != bgfx::kInvalidHandle));

            bgfx::setUniform(h, data, size);
        }

        void Destroy()
        {
            for (auto p : mPrograms)
            {
                bgfx::destroy(p.second.pgh);
            }

            for (auto u : mUniforms)
            {
                bgfx::destroy(u.second);
            }

            mRenderObjects.destroy();
            bgfx::shutdown();
        }

        void PreUpdate(const RendererUpdateContext &ctx)
        {
            if (ctx.width != mFrameBufferWidth || ctx.height != mFrameBufferHeight)
            {
                mFrameBufferWidth = ctx.width;
                mFrameBufferHeight = ctx.height;

                bgfx::reset(mFrameBufferWidth, (uint32_t)mFrameBufferHeight, BGFX_RESET_VSYNC);
                bgfx::setViewRect(mViewId, 0, 0, (uint16_t)mFrameBufferWidth, (uint16_t)mFrameBufferHeight);
            }

            {
                float view[16];
                float proj[16];

                bx::Vec3 eye = {mCamera->position.x(), mCamera->position.y(), mCamera->position.z()};
                bx::Vec3 at = {mCamera->position.x() + mCamera->forward.x(),
                               mCamera->position.y() + mCamera->forward.y(),
                               mCamera->position.z() + mCamera->forward.z()};
                bx::Vec3 upArr = {mCamera->up.x(), mCamera->up.y(), mCamera->up.z()};

                bx::mtxLookAt(view, eye, at, upArr);

                float aspect = (mWindowPtr->mHeight > 0) ? (float)mWindowPtr->mWidth / (float)mWindowPtr->mHeight : 1.0f;
                const bgfx::Caps *caps = bgfx::getCaps();
                float nearp = 0.1f;
                float farp = 100.f;
                float nf[4] = {nearp, farp, 0, 0};
                bx::mtxProj(proj, 60.0f, aspect, nearp, farp, caps->homogeneousDepth);
                bgfx::setViewTransform(mViewId, view, proj);

                // this is not right
                float mtx[16];
                bx::mtxIdentity(mtx);
                // this is not right

                bgfx::touch(mViewId);
                bgfx::setTransform(mtx);
                UpdateUniform("u_camera", nf, 1);
            }
        }

        void Update(const RendererUpdateContext &ctx)
        {
            for (auto o : mFrameObjects.objs)
            {
                if (o.line)
                    bgfx::setState(
                        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_LINES);
                else
                    bgfx::setState(
                        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA);

                auto b = mRenderObjects.robjs[o.h];
                Mat4f mtx = o.transform.matrix();
                auto p = mPrograms.find(o.p);
                bgfx::setVertexBuffer(0, b.vbh);
                bgfx::setTransform(mtx.data());
                bgfx::submit(mViewId, p->second.pgh);
            }
        }

        void PostUpdate(const RendererUpdateContext &ctx)
        {
            bgfx::frame();
            mFrameObjects.objs.clear();
        }

    private:
        void initDefaultUniforms()
        {
            CreateUniform<lmcore::Vec4f>("u_camera");
        }

    private:
        std::unordered_map<std::string, RProgram> mPrograms;
        std::unordered_map<std::string, bgfx::UniformHandle> mUniforms;

        FrameObjectContainer mFrameObjects;
        RenderObjectContainer mRenderObjects;
        std::shared_ptr<Window> mWindowPtr = nullptr;

        uint32_t mFrameBufferWidth = 0u;
        uint32_t mFrameBufferHeight = 0u;
        const bgfx::ViewId mViewId = 0;

        bgfx::VertexLayout mPosColorLayout;

        // Camera mCamera;
        std::shared_ptr<Camera> mCamera;
        Controller mController;
        double mLastTime;
        double mCurrentTime;
    };

    Renderer::Renderer(std::shared_ptr<Window> wptr) : impl(std::make_unique<Impl>(wptr))
    {
    }

    Renderer::~Renderer()
    {
    }

    bool Renderer::Init()
    {
        return impl->Init();
    }

    void Renderer::SetCamera(std::shared_ptr<Camera> camptr)
    {
        impl->SetCamera(camptr);
    }

    void Renderer::PreUpdate(const RendererUpdateContext &ctx)
    {
        impl->PreUpdate(ctx);
    }

    void Renderer::Update(const RendererUpdateContext &ctx)
    {
        impl->Update(ctx);
    }

    void Renderer::PostUpdate(const RendererUpdateContext &ctx)
    {
        impl->PostUpdate(ctx);
    }

    void Renderer::Destroy()
    {
        impl->Destroy();
    }

    bool Renderer::CreateProgram(const std::string &name, const std::string &shadern)
    {
        return impl->CreateProgram(name, shadern);
    }

    void Renderer::PushFrameObject(FrameObject obj)
    {
        impl->PushFrameObject(obj);
    }

    void Renderer::PushFrameObjects(const std::vector<FrameObject> &objs)
    {
        impl->PushFrameObjects(objs);
    }

    RenderObjectHandle Renderer::CreateRenderObject(const lmcore::PosColorVertex *const vertices, uint32_t count)
    {
        return impl->CreateRenderObject(vertices, count);
    }
}