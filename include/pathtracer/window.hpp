#pragma once

#define GLFW_INCLUDE_NONE

#include <filesystem>
#include <GLFW/glfw3.h>

namespace path_tracer
{
    struct WindowState
    {
        int pos_x;
        int pos_y;
        int width;
        int height;
    };

    class Window
    {
    public:
        Window(int width, int height, const std::string &title, const std::filesystem::path &icon);

        ~Window();

        [[nodiscard]] GLFWwindow *Handle() const;

        [[nodiscard]] bool Frame() const;

        void MakeContextCurrent() const;

        void GetFrameBufferSize(int &width, int &height) const;

    private:
        GLFWwindow *m_Handle;
    };
}
