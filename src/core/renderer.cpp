#include "core/renderer.h"
#include "renderer.h"

namespace lmv
{

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
        return RenderObjectHandle(robjs.size()-1);
    }

    void destroy(RenderObjectHandle handle)
    {
        assert(handle < robjs.size());
        auto & o = robjs[handle];
        if(o.destroyed)
            return;
        bgfx::destroy(o.vbh);
        if(o.indexDraw)
            bgfx::destroy(o.ibh);
        o.destroyed = true;
    }

    void destroy()
    {
        for(auto & o : robjs)
        {
            if(o.destroyed)
                continue;
            bgfx::destroy(o.vbh);
            if(o.indexDraw)
                bgfx::destroy(o.ibh);
            o.destroyed = true;
        }
    }

    std::vector<RObject> robjs; 
};

class Renderer::Impl{
    public:
        Impl(std::shared_ptr<Window> wptr) : mWindowPtr(wptr)
        {

        }

        bool Init()
        {
            //todo use safe init
            bgfx::Init init;
            init.type = bgfx::RendererType::OpenGL;
            init.vendorId = BGFX_PCI_ID_NONE;
            init.platformData.ndt = mWindowPtr->x11Display;
            init.platformData.nwh = (void*)(uintptr_t)(mWindowPtr->x11Window);
            init.platformData.context = nullptr;
            init.platformData.backBuffer = nullptr;
            init.platformData.backBufferDS = nullptr;

            init.resolution.width  = (uint32_t)mWindowPtr->mWidth;
            init.resolution.height = (uint32_t)mWindowPtr->mHeight;
            init.resolution.reset  = BGFX_RESET_VSYNC;

            if(!bgfx::init(init))
                return false;
            bgfx::setViewClear(mViewId,BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.f, 0);
            bgfx::setViewRect(mViewId, 0, 0, (uint16_t)mWindowPtr->mWidth, (uint16_t)mWindowPtr->mHeight);

            s_PosColorLayout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float, true)
                .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Float, true)
                .end();

            mLastTime = glfwGetTime();
            return true;
        }

        bool CreateProgram(const std::string & name, const std::string & shadern)
        {
            auto shaderpath = getShaderPath();
            bgfx::ShaderHandle vsh = loadShaderBin((shaderpath + "/vs_" + shadern + ".bin").c_str());
            bgfx::ShaderHandle fsh = loadShaderBin((shaderpath + "/fs_" + shadern + ".bin").c_str());
            if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
            {
                std::cerr << "Invalid shader handles!" << std::endl;
                return false;
            }

            if(mPrograms.find("name")!=mPrograms.end())
            {
                std::cerr << "Program name is used already!" << std::endl;
                return false;
            }

            auto pgh = bgfx::createProgram(vsh, fsh, true);

            mPrograms.insert({name,RProgram{.vsh = vsh, .fsh = fsh, .pgh = pgh}});
            return true;
        }

        RenderObjectHandle CreateRenderObject(const std::vector<lmcore::PosColorVertex> & vertices)
        {
            auto vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices.data(),vertices.size() * sizeof(lmcore::PosColorVertex)),mPosColorLayout);
            RObject obj{.vbh = vbh};
            return mRenderObjects.add(obj);
        }

        template<typename T>
        void CreateUniform(const std::string & name)
        {
            assert(mUniforms.find(name)==mUniforms.end());
            bgfx::UniformHandle handle;
            if constexpr(std::is_same_v(T,bgfx::UniformType::Vec4))
            {
                handle = bgfx::createUniform("u_camera", bgfx::UniformType::Vec4);
            }
            else
            {

            }

            mUniforms.insert({name,handle});
        }

        void Destroy()
        {
            for(auto p : mPrograms)
            {
                bgfx::destroy(p.second.pgh);
            }

            for(auto u : mUniforms)
            {
                bgfx::destroy(u.second);
            }

            mRenderObjects.destroy();
        }

        bool ShouldClose()
        {
            return !glfwWindowShouldClose(mWindowPtr->mWindow);
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

            //TODO move to camera/controller
            {
                float moveForward = 0.0f;
                float moveRight   = 0.0f;
                float moveUp      = 0.0f;

                auto window = mWindowPtr->mWindow;
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

                        mCamera.yaw   -= (float)dx * mController.mouseSensitivity;
                        mCamera.pitch -= (float)dy * mController.mouseSensitivity;

                        const float limit = bx::toRad(89.5f);
                        if (mCamera.pitch >  limit) mCamera.pitch =  limit;
                        if (mCamera.pitch < -limit) mCamera.pitch = -limit;
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
                    forward.z() * moveForward + right.z() * moveRight + up.z() * moveUp
                };

                if (moveForward != 0.0f || moveRight != 0.0f || moveUp != 0.0f)
                {
                    moveDir.normalize();
                    mCamera.position = mCamera.position + moveDir * (mController.moveSpeed * dt);
                }

                float view[16];
                float proj[16];

                bx::Vec3 eye = { mCamera.position.x(), mCamera.position.y(), mCamera.position.z() };
                bx::Vec3 at  = { mCamera.position.x() + forward.x(),
                                 mCamera.position.y() + forward.y(),
                                 mCamera.position.z() + forward.z() };
                bx::Vec3 upArr = { up.x(), up.y(), up.z() };

                bx::mtxLookAt(view, eye, at, upArr);

                float aspect = (mWindowPtr->mHeight > 0) ? (float)mWindowPtr->mWidth / (float)mWindowPtr->mHeight : 1.0f;
                const bgfx::Caps* caps = bgfx::getCaps();
                float nearp = 0.1f;
                float farp = 100.f;
                float nf[4] = {nearp, farp, 0, 0};
                bx::mtxProj(proj, 60.0f, aspect, nearp, farp, caps->homogeneousDepth);
                bgfx::setViewTransform(mViewId, view, proj);
                
                //this is not right
                float mtx[16];
                bx::mtxIdentity(mtx);
                //this is not right
                
                bgfx::touch(mViewId);
                bgfx::setTransform(mtx);
                bgfx::setUniform(u_camera, nf);
            }
        }

    private:
        std::unordered_map<std::string, RProgram> mPrograms;
        std::unordered_map<std::string, bgfx::UniformHandle> mUniforms;
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
}