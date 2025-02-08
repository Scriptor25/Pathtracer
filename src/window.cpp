#include <iostream>
#include <stb_image.h>
#include <pathtracer/window.hpp>

void glfwErrorCallback(const int error_code, const char *description)
{
    std::cerr << "[GLFW 0x" << std::hex << error_code << std::dec << "] " << description << std::endl;
}

void toggle_screen_mode(GLFWwindow *window)
{
    auto state = static_cast<path_tracer::WindowState *>(glfwGetWindowUserPointer(window));
    if (state)
    {
        glfwSetWindowMonitor(
            window,
            nullptr,
            state->pos_x,
            state->pos_y,
            state->width,
            state->height,
            GLFW_DONT_CARE);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);

        glfwSetWindowUserPointer(window, nullptr);
        delete state;
    }
    else
    {
        state = new path_tracer::WindowState();
        glfwGetWindowPos(window, &state->pos_x, &state->pos_y);
        glfwGetWindowSize(window, &state->width, &state->height);
        glfwSetWindowUserPointer(window, state);

        const auto pMonitor = glfwGetPrimaryMonitor();
        const auto pMode = glfwGetVideoMode(pMonitor);
        glfwSetWindowMonitor(window, nullptr, 0, 0, pMode->width, pMode->height, pMode->refreshRate);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
    }
}

void glfwKeyCallback(GLFWwindow *pWindow, const int key, const int /*scancode*/, const int action, const int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(pWindow, GLFW_TRUE);

    if (key == GLFW_KEY_F11 && action == GLFW_RELEASE)
        toggle_screen_mode(pWindow);
}

path_tracer::Window::Window(
    const int width,
    const int height,
    const std::string &title,
    const std::filesystem::path &icon)
{
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
        throw std::runtime_error("failed to initialize glfw");

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);

    m_Handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Handle)
        throw std::runtime_error("failed to create window");

    glfwMakeContextCurrent(m_Handle);
    glfwSetKeyCallback(m_Handle, glfwKeyCallback);
    glfwSwapInterval(1);

    GLFWimage image;
    image.pixels = stbi_load(icon.string().c_str(), &image.width, &image.height, nullptr, 4);
    glfwSetWindowIcon(m_Handle, 1, &image);
    stbi_image_free(image.pixels);
}

path_tracer::Window::~Window()
{
    glfwDestroyWindow(m_Handle);
    glfwTerminate();
}

GLFWwindow *path_tracer::Window::Handle() const
{
    return m_Handle;
}

bool path_tracer::Window::Frame() const
{
    glfwSwapBuffers(m_Handle);
    glfwPollEvents();

    return !glfwWindowShouldClose(m_Handle);
}

void path_tracer::Window::MakeContextCurrent() const
{
    glfwMakeContextCurrent(m_Handle);
}

void path_tracer::Window::GetFrameBufferSize(int &width, int &height) const
{
    glfwGetFramebufferSize(m_Handle, &width, &height);
}
