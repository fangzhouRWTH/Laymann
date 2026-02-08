#include "core/framework.h"

namespace lmcore
{
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
        Framework::Context *ctx = reinterpret_cast<Framework::Context *>(glfwGetWindowUserPointer(window));
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
        Framework::Context *ctx = reinterpret_cast<Framework::Context *>(glfwGetWindowUserPointer(window));
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
        Framework::Context *ctx = reinterpret_cast<Framework::Context *>(glfwGetWindowUserPointer(window));
        auto &ctrl = ctx->ctrl_ptr->current_mouse_states;
        ctrl.lastMouseX = xpos;
        ctrl.lastMouseY = ypos;

        return;
    }

    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        Framework::Context *ctx = reinterpret_cast<Framework::Context *>(glfwGetWindowUserPointer(window));
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

        // mControl->previous_key_states = mControl->current_key_states;

        // TODO CALL BACK FUNC
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(mWindow);
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
        {
            glfwTerminate();
            return false;
        }

        glfwSetWindowUserPointer(mctx.win_ptr->mWindow, &mctx);

        glfwSetKeyCallback(mctx.win_ptr->mWindow, key_callback);
        glfwSetMouseButtonCallback(mctx.win_ptr->mWindow, mouse_button_callback);
        glfwSetCursorPosCallback(mctx.win_ptr->mWindow, cursor_callback);
        glfwSetScrollCallback(mctx.win_ptr->mWindow, scroll_callback);

        return true;
    }
    bool Framework::ShouldClose()
    {
        return mctx.win_ptr->ShouldClose();
    }
    void Framework::Terminate()
    {
        mctx.win_ptr->Terminate();
    }
    void Framework::PreUpdate()
    {
        clock.tick();
        // todo
        mctx.ctrl_ptr->current_key_states.swap(mctx.ctrl_ptr->previous_key_states);
        mctx.ctrl_ptr->current_key_states.clear();
        mctx.ctrl_ptr->previous_mouse_states = mctx.ctrl_ptr->current_mouse_states;
        mctx.ctrl_ptr->current_mouse_states.frame();
        glfwPollEvents();
        mctx.win_ptr->Update();
    }
    void Framework::Update()
    {
    }
    void Framework::PostUpdate()
    {
    }
    Framework::Context Framework::GetContext()
    {
        return mctx;
    }
    void Framework::GetFrameSize(uint32_t &width, uint32_t& height)
    {
        if (!mctx.win_ptr)
        {
            width = 0u;
            height = 0u;
            return;
        }
        width = mctx.win_ptr->mWidth;
        height = mctx.win_ptr->mHeight;
    }
}