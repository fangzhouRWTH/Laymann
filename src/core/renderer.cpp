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

    private:
        std::unordered_map<std::string, RProgram> mPrograms;
        std::unordered_map<std::string, bgfx::UniformHandle> mUniforms;
        RenderObjectContainer mRenderObjects;
        std::shared_ptr<Window> mWindowPtr = nullptr;
        const bgfx::ViewId mViewId = 0;

        bgfx::VertexLayout mPosColorLayout;
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