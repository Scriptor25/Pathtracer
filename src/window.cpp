#include <iostream>
#include <pathtracer/window.hpp>

void glfwErrorCallback(const int errorCode, const char* pDescription)
{
    std::cerr << "[GLFW 0x" << std::hex << errorCode << std::dec << "] " << pDescription << std::endl;
}

void glfwToggleFullscreen(GLFWwindow* pWindow)
{
    auto pState = static_cast<pathtracer::WindowState*>(glfwGetWindowUserPointer(pWindow));
    if (pState)
    {
        glfwSetWindowMonitor(pWindow, nullptr, pState->xpos, pState->ypos, pState->width, pState->height, GLFW_DONT_CARE);
        glfwSetWindowAttrib(pWindow, GLFW_RESIZABLE, GLFW_TRUE);

        glfwSetWindowUserPointer(pWindow, nullptr);
        delete pState;
    }
    else
    {
        pState = new pathtracer::WindowState();
        glfwGetWindowPos(pWindow, &pState->xpos, &pState->ypos);
        glfwGetWindowSize(pWindow, &pState->width, &pState->height);
        glfwSetWindowUserPointer(pWindow, pState);

        const auto pMonitor = glfwGetPrimaryMonitor();
        const auto pMode = glfwGetVideoMode(pMonitor);
        glfwSetWindowMonitor(pWindow, nullptr, 0, 0, pMode->width, pMode->height, pMode->refreshRate);
        glfwSetWindowAttrib(pWindow, GLFW_RESIZABLE, GLFW_FALSE);
    }
}

void glfwKeyCallback(GLFWwindow* pWindow, const int key, const int /*scancode*/, const int action, const int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(pWindow, GLFW_TRUE);

    if (key == GLFW_KEY_F11 && action == GLFW_RELEASE)
        glfwToggleFullscreen(pWindow);
}

pathtracer::Window::Window(const int width, const int height)
{
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
        throw std::runtime_error("glfwInit");

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);

    m_GLFW = glfwCreateWindow(width, height, "Pathtracer", nullptr, nullptr);
    if (!m_GLFW)
        throw std::runtime_error("glfwCreateWindow");

    glfwMakeContextCurrent(m_GLFW);
    glfwSetKeyCallback(m_GLFW, glfwKeyCallback);
    glfwSwapInterval(1);
}

pathtracer::Window::~Window()
{
    glfwDestroyWindow(m_GLFW);
    glfwTerminate();
}

GLFWwindow* pathtracer::Window::GLFW() const
{
    return m_GLFW;
}

bool pathtracer::Window::Frame() const
{
    glfwSwapBuffers(m_GLFW);
    glfwPollEvents();

    return !glfwWindowShouldClose(m_GLFW);
}

void pathtracer::Window::MakeContextCurrent() const
{
    glfwMakeContextCurrent(m_GLFW);
}

void pathtracer::Window::GetFramebufferSize(int* pWidth, int* pHeight) const
{
    glfwGetFramebufferSize(m_GLFW, pWidth, pHeight);
}
