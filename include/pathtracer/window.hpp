#pragma once

#include <GLFW/glfw3.h>

namespace pathtracer
{
    struct WindowState
    {
        int xpos;
        int ypos;
        int width;
        int height;
    };

    class Window
    {
    public:
        Window(int width, int height);
        ~Window();

        [[nodiscard]] GLFWwindow* GLFW() const;

        [[nodiscard]] bool Frame() const;
        void MakeContextCurrent() const;

        void GetFramebufferSize(int* pWidth, int* pHeight) const;

    private:
        GLFWwindow* m_GLFW;
    };
}
