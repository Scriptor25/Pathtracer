#include <iostream>
#include <stb_image.h>
#include <pathtracer/window.hpp>

static void glfw_error_callback(const int error_code, const char *description)
{
    std::cerr << "[GLFW 0x" << std::hex << error_code << std::dec << "] " << description << std::endl;
}

static void glfw_key_callback(
    GLFWwindow *window,
    const int key,
    const int scancode,
    const int action,
    const int mods)
{
    static_cast<pathtracer::Window *>(glfwGetWindowUserPointer(window))->OnKey(key, scancode, action, mods);
}

static void glfw_frame_buffer_size_callback(GLFWwindow *window, const int width, const int height)
{
    static_cast<pathtracer::Window *>(glfwGetWindowUserPointer(window))->OnFramebufferSize(width, height);
}

static unsigned window_count = 0;

static void initialize()
{
    if (window_count++ == 0)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            throw std::runtime_error("failed to initialize glfw");
    }
}

static void terminate()
{
    if (--window_count == 0)
    {
        glfwTerminate();
    }
}

pathtracer::Window::Window(
    const int width,
    const int height,
    const std::string &title,
    const std::filesystem::path &icon)
{
    initialize();

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);

    m_Handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Handle)
        throw std::runtime_error("failed to create window");

    glfwMakeContextCurrent(m_Handle);
    glfwSetWindowUserPointer(m_Handle, this);
    glfwSetKeyCallback(m_Handle, glfw_key_callback);
    glfwSetFramebufferSizeCallback(m_Handle, glfw_frame_buffer_size_callback);
    glfwSwapInterval(1);

    GLFWimage image;
    image.pixels = stbi_load(icon.string().c_str(), &image.width, &image.height, nullptr, 4);
    glfwSetWindowIcon(m_Handle, 1, &image);
    stbi_image_free(image.pixels);
}

pathtracer::Window::~Window()
{
    if (!m_Handle)
        return;

    glfwDestroyWindow(m_Handle);
    terminate();
}

pathtracer::Window::Window(Window &&other) noexcept
    : m_Handle(other.m_Handle),
      m_State(other.m_State)
{
    other.m_Handle = nullptr;
}

pathtracer::Window &pathtracer::Window::operator=(Window &&other) noexcept
{
    std::swap(m_Handle, other.m_Handle);
    std::swap(m_State, other.m_State);
    return *this;
}

GLFWwindow *pathtracer::Window::Handle() const
{
    return m_Handle;
}

void pathtracer::Window::SetFramebufferSizeCallback(const Callback &callback)
{
    m_FramebufferSizeCallback = callback;
}

bool pathtracer::Window::Spin() const
{
    glfwSwapBuffers(m_Handle);
    glfwPollEvents();

    return !glfwWindowShouldClose(m_Handle);
}

void pathtracer::Window::MakeContextCurrent() const
{
    glfwMakeContextCurrent(m_Handle);
}

void pathtracer::Window::GetFramebufferSize(int &width, int &height) const
{
    glfwGetFramebufferSize(m_Handle, &width, &height);
}

void pathtracer::Window::OnKey(const int key, int scancode, const int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(m_Handle, GLFW_TRUE);

    if (key == GLFW_KEY_F11 && action == GLFW_RELEASE)
        ToggleScreenMode();
}

void pathtracer::Window::OnFramebufferSize(const int width, const int height) const
{
    if (m_FramebufferSizeCallback)
        m_FramebufferSizeCallback();
}

void pathtracer::Window::ToggleScreenMode()
{
    if (m_State.Init)
    {
        m_State.Init = false;
        glfwSetWindowMonitor(
            m_Handle,
            nullptr,
            m_State.PositionX,
            m_State.PositionY,
            m_State.Width,
            m_State.Height,
            GLFW_DONT_CARE);
        return;
    }

    m_State.Init = true;
    glfwGetWindowPos(m_Handle, &m_State.PositionX, &m_State.PositionY);
    glfwGetWindowSize(m_Handle, &m_State.Width, &m_State.Height);

    const auto monitor = glfwGetPrimaryMonitor();
    const auto video_mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(m_Handle, monitor, 0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
}
