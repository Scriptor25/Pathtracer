#pragma once

#define GLFW_INCLUDE_NONE

#include <filesystem>
#include <functional>
#include <GLFW/glfw3.h>

namespace pathtracer
{
    struct WindowState
    {
        bool Init = false;
        int PositionX = 0;
        int PositionY = 0;
        int Width = 0;
        int Height = 0;
    };

    using Callback = std::function<void()>;

    class Window
    {
    public:
        Window() = default;
        Window(
            int width,
            int height,
            const std::string &title,
            const std::filesystem::path &icon);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        Window(Window &&other) noexcept;
        Window &operator=(Window &&other) noexcept;

        [[nodiscard]] GLFWwindow *Handle() const;

        void SetFramebufferSizeCallback(const Callback &callback);
        [[nodiscard]] bool Spin() const;

        void MakeContextCurrent() const;
        void GetFramebufferSize(int &width, int &height) const;

        void OnKey(int key, int scancode, int action, int mods);
        void OnFramebufferSize(int width, int height) const;

        void ToggleScreenMode();

    private:
        GLFWwindow *m_Handle = nullptr;
        WindowState m_State;
        Callback m_FramebufferSizeCallback;
    };
}
