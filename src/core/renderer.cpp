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

        bool ShouldClose()
        {
            return glfwWindowShouldClose(mWindowPtr->mWindow);
        }

        void PreUpdate()
        {
            mCurrentTime = glfwGetTime();
            float dt = (float)(mCurrentTime - mLastTime);
            mLastTime = mCurrentTime;

            glfwPollEvents();

            int fbW, fbH;
            glfwGetFramebufferSize(mWindowPtr->mWindow, &fbW, &fbH);
            if (fbW != mWindowPtr->mWidth || fbH != mWindowPtr->mHeight)
            {
                mWindowPtr->mWidth = fbW;
                mWindowPtr->mHeight = fbH;
                bgfx::reset((uint32_t)fbW, (uint32_t)fbH, BGFX_RESET_VSYNC);
                bgfx::setViewRect(mViewId, 0, 0, (uint16_t)fbW, (uint16_t)fbW);
            }

            // TODO move to camera/controller
            {
                float moveForward = 0.0f;
                float moveRight = 0.0f;
                float moveUp = 0.0f;

                auto window = mWindowPtr->mWindow;
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    moveForward += 1.0f;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    moveForward -= 1.0f;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    moveRight -= 1.0f;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    moveRight += 1.0f;
                if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                    moveUp += 1.0f;
                if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                    moveUp -= 1.0f;

                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                {
                    double mx, my;
                    glfwGetCursorPos(window, &mx, &my);

                    if (!mController.rotating)
                    {
                        mController.rotating = true;
                        mController.lastMouseX = mx;
                        mController.lastMouseY = my;
                    }
                    else
                    {
                        double dx = mx - mController.lastMouseX;
                        double dy = my - mController.lastMouseY;
                        mController.lastMouseX = mx;
                        mController.lastMouseY = my;

                        mCamera.yaw -= (float)dx * mController.mouseSensitivity;
                        mCamera.pitch -= (float)dy * mController.mouseSensitivity;

                        const float limit = bx::toRad(89.5f);
                        if (mCamera.pitch > limit)
                            mCamera.pitch = limit;
                        if (mCamera.pitch < -limit)
                            mCamera.pitch = -limit;
                    }
                }
                else
                {
                    mController.rotating = false;
                }

                lmcore::Vec3f forward{
                    -std::sin(mCamera.yaw) * std::cos(mCamera.pitch),
                    -std::cos(mCamera.yaw) * std::cos(mCamera.pitch),
                    std::sin(mCamera.pitch),
                };
                forward.normalize();

                lmcore::Vec3f worldUp{0.0f, 0.0f, 1.0f};
                lmcore::Vec3f right = forward.cross(worldUp);
                right.normalize();
                lmcore::Vec3f up = right.cross(forward);

                lmcore::Vec3f moveDir{
                    forward.x() * moveForward + right.x() * moveRight + up.x() * moveUp,
                    forward.y() * moveForward + right.y() * moveRight + up.y() * moveUp,
                    forward.z() * moveForward + right.z() * moveRight + up.z() * moveUp};

                if (moveForward != 0.0f || moveRight != 0.0f || moveUp != 0.0f)
                {
                    moveDir.normalize();
                    mCamera.position = mCamera.position + moveDir * (mController.moveSpeed * dt);
                }

                float view[16];
                float proj[16];

                bx::Vec3 eye = {mCamera.position.x(), mCamera.position.y(), mCamera.position.z()};
                bx::Vec3 at = {mCamera.position.x() + forward.x(),
                               mCamera.position.y() + forward.y(),
                               mCamera.position.z() + forward.z()};
                bx::Vec3 upArr = {up.x(), up.y(), up.z()};

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

        void Update()
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

        void PostUpdate()
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
        const bgfx::ViewId mViewId = 0;

        bgfx::VertexLayout mPosColorLayout;

        Camera mCamera;
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

    bool Renderer::ShouldClose()
    {
        return impl->ShouldClose();
    }

    void Renderer::PreUpdate()
    {
        impl->PreUpdate();
    }

    void Renderer::Update()
    {
        impl->Update();
    }

    void Renderer::PostUpdate()
    {
        impl->PostUpdate();
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

#define ACTION_CASE(STATE, ACTION) \
    case ACTION:                   \
    {                              \
        s.set(STATE);              \
        break;                     \
    }

#define KEY_CASE(KEY, GKEY) \
    case GKEY:              \
    {                       \
        ks.set(KEY, s);     \
        break;              \
    }

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        Framework::Context *ctx = reinterpret_cast<Framework::Context *>(window);
        auto &ks = ctx->ctrl_ptr->current_key_states;
        KeyState s;

        switch (action)
        {
            ACTION_CASE(EKeyState::Press, GLFW_PRESS)
            ACTION_CASE(EKeyState::Repeat, GLFW_REPEAT)
        default:
            break;
        }

        switch (key)
        {
            KEY_CASE(EKey::ESC, GLFW_KEY_ESCAPE)
            KEY_CASE(EKey::W, GLFW_KEY_W)
            KEY_CASE(EKey::S, GLFW_KEY_S)
            KEY_CASE(EKey::D, GLFW_KEY_D)
            KEY_CASE(EKey::A, GLFW_KEY_A)
            KEY_CASE(EKey::E, GLFW_KEY_E)
            KEY_CASE(EKey::Q, GLFW_KEY_Q)
        default:
            break;
        }
    }

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        Framework::Context *ctx = reinterpret_cast<Framework::Context *>(window);
        auto &ks = ctx->ctrl_ptr->current_key_states;
        KeyState s;

        switch (action)
        {
            ACTION_CASE(EKeyState::Press, GLFW_PRESS)
        default:
            break;
        }
        switch (button)
        {
            KEY_CASE(EKey::MouseRight, GLFW_MOUSE_BUTTON_RIGHT)
        default:
            break;
        }
    }

    void cursor_callback(GLFWwindow *window, double xpos, double ypos)
    {
        Framework::Context *ctx = reinterpret_cast<Framework::Context *>(window);
        auto & ctrl = ctx->ctrl_ptr->current_mouse_states;
        ctrl.lastMouseX = xpos;
        ctrl.lastMouseY = ypos;
    }

    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
    }

    bool Window::Init()
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        mWindow = glfwCreateWindow(mWidth, mHeight, "Laymann v 0 0 1", nullptr, nullptr);
        if (mWindow == nullptr)
        {
            std::fprintf(stderr, "Failed to create GLFW window\n");
            return false;
        }
        glfwMakeContextCurrent(mWindow);
        glfwSwapInterval(0);
        // todo specify platform
        x11Display = glfwGetX11Display();
        x11Window = glfwGetX11Window(mWindow);
        return true;
    }

    void Window::Update()
    {
        int fbW, fbH;
        glfwGetFramebufferSize(mWindow, &fbW, &fbH);
        if (fbW != mWidth || fbH != mHeight)
        {
            mWidth = fbW;
            mHeight = fbH;
        }

        mControl->previous_key_states = mControl->current_key_states;

        // TODO CALL BACK FUNC
    }

    void Window::Terminate()
    {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    bool Framework::Init(const Info &info)
    {
        mctx.win_ptr = std::make_shared<Window>(info.win_width, info.win_height);
        mctx.ctrl_ptr = std::make_shared<Control>();
        mctx.cam_ptr = std::make_shared<Camera>();

        if (!glfwInit())
        {
            std::fprintf(stderr, "Failed to initialize GLFW\n");
            return false;
        }

        if (!mctx.win_ptr->Init())
            glfwTerminate();

        glfwSetWindowUserPointer(mctx.win_ptr->mWindow, &mctx);

        glfwSetKeyCallback(mctx.win_ptr->mWindow, key_callback);
        glfwSetMouseButtonCallback(mctx.win_ptr->mWindow, mouse_button_callback);
        glfwSetCursorPosCallback(mctx.win_ptr->mWindow, cursor_callback);
        glfwSetScrollCallback(mctx.win_ptr->mWindow, scroll_callback);
    }
    void Framework::PreUpdate()
    {
        //todo
        mctx.ctrl_ptr->current_key_states.swap(mctx.ctrl_ptr->previous_key_states);
        mctx.ctrl_ptr->current_key_states.clear();
        mctx.ctrl_ptr->previous_mouse_states = mctx.ctrl_ptr->current_mouse_states;
        mctx.ctrl_ptr->current_mouse_states.frame();
        glfwPollEvents();
    }
    void Framework::Update()
    {
        mctx.win_ptr->Update();
    }
    void Framework::PostUpdate()
    {
    }
}