#pragma once

#include <glm/glm.hpp>
#include <pathtracer/buffer.hpp>
#include <pathtracer/scene.hpp>
#include <pathtracer/shader.hpp>
#include <pathtracer/vertex_array.hpp>
#include <pathtracer/window.hpp>

namespace pathtracer
{
    class App
    {
    public:
        App();
        ~App();

        void OnStart();
        void OnFrame();

    private:
        std::filesystem::path m_Assets;
        std::unique_ptr<Window> m_Window;

        std::unique_ptr<Scene> m_Scene;

        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<VertexArray> m_VertexArray;
        std::unique_ptr<Buffer> m_VertexBuffer;

        GLuint m_AccumulationTexture{};

        unsigned m_SampleCount = 1u;
        int m_PreviousWidth = 0;
        int m_PreviousHeight = 0;

        static constexpr GLfloat VERTICES[]{-1.f, -1.f, -1.f, 1.f, 1.f, 1.f, 1.f, -1.f};
        static constexpr GLuint INDICES[]{0u, 1u, 2u, 2u, 3u, 0u};
        static constexpr glm::vec3 ORIGIN{0.f, 0.f, 3.75f};
        static constexpr glm::vec3 TARGET{0.f, 0.f, 0.f};
        static constexpr glm::vec3 UP{0.f, 1.f, 0.f};

        static glm::mat4 CAMERA_TO_WORLD;
    };
}
