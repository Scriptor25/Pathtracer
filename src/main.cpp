#define GLFW_INCLUDE_NONE

#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <assimp/postprocess.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <pathtracer/buffer.hpp>
#include <pathtracer/scene.hpp>
#include <pathtracer/shader.hpp>
#include <pathtracer/vertex_array.hpp>
#include <pathtracer/window.hpp>

void gl_debug_message_callback(
    const GLenum /*source*/,
    const GLenum /*type*/,
    const GLuint id,
    const GLenum severity,
    const GLsizei /*length*/,
    const GLchar *pMessage,
    const void * /*pUserParam*/)
{
    if (severity != GL_DEBUG_SEVERITY_HIGH)
        return;
    std::cerr << "[GL 0x" << std::hex << id << std::dec << "] " << pMessage << std::endl;
}

int main()
{
    const auto assets = std::filesystem::canonical("assets");

    const path_tracer::Window window(600, 600, "PathTracer", assets / "icon.png");

    if (const auto error = glewInit())
    {
        std::cerr
                << "[GLEW 0x"
                << std::hex
                << error
                << std::dec
                << "] failed to initialize glew: "
                << glewGetErrorString(error)
                << std::endl;
        return 1;
    }

    glDebugMessageCallback(gl_debug_message_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glClearColor(0.2f, 0.3f, 1.0f, 1.0f);

    ImGui::CreateContext();
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigDockingTransparentPayload = true;
    io.ConfigViewportsNoTaskBarIcon = true;

    ImGui_ImplOpenGL3_Init();
    ImGui_ImplGlfw_InitForOpenGL(window.Handle(), true);

    constexpr GLfloat VERTICES[]{-1.f, -1.f, -1.f, 1.f, 1.f, 1.f, 1.f, -1.f};
    constexpr GLuint INDICES[]{0u, 1u, 2u, 2u, 3u, 0u};

    const path_tracer::VertexArray vertex_array;
    vertex_array.Bind();
    const path_tracer::Buffer vertex_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    vertex_buffer.Bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    vertex_buffer.Data(sizeof(VERTICES), VERTICES);
    vertex_buffer.Unbind();
    vertex_array.Unbind();

    GLuint accumulation_texture;
    glGenTextures(1, &accumulation_texture);

    const path_tracer::Shader shader(assets / "main.yaml");

    path_tracer::Scene scene;
    scene.LoadModel(assets / "objects" / "cornell_box.obj", aiProcess_GenNormals);

    // scene.LoadModel(assets / "objects" / "axis.obj");
    // scene.GetLastModel().Transform = scale(rotate(translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.95f, 0.0f)), glm::radians(45.0f), normalize(glm::vec3(0.0f, 1.0f, 0.0f))), glm::vec3(0.1f));
    // scene.GetLastModel().InverseTransform = inverse(scene.GetLastModel().Transform);
    // scene.GetLastModel().NormalTransform = transpose(scene.GetLastModel().InverseTransform);

    scene.LoadModel(assets / "objects" / "cow.obj", aiProcess_GenSmoothNormals);
    scene.GetLastModel().Transform = scale(
        rotate(
            translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.65f, 0.0f)),
            glm::radians(-135.0f),
            normalize(glm::vec3(0.0f, 1.0f, 0.0f))),
        glm::vec3(0.1f));
    scene.GetLastModel().InverseTransform = inverse(scene.GetLastModel().Transform);
    scene.GetLastModel().NormalTransform = transpose(scene.GetLastModel().InverseTransform);

    scene.LoadModel(assets / "objects" / "teapot.obj", aiProcess_GenSmoothNormals);
    scene.GetLastModel().Transform = scale(
        rotate(
            translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.0f, 0.0f)),
            glm::radians(-45.0f),
            normalize(glm::vec3(0.0f, 1.0f, 0.0f))),
        glm::vec3(0.2f));
    scene.GetLastModel().InverseTransform = inverse(scene.GetLastModel().Transform);
    scene.GetLastModel().NormalTransform = transpose(scene.GetLastModel().InverseTransform);

    scene.Upload();

    constexpr glm::vec3 origin(0.f, 0.f, 3.75f);
    constexpr glm::vec3 target(0.f, 0.f, 0.f);
    constexpr glm::vec3 up(0.f, 1.f, 0.f);
    auto camera_to_world = inverse(lookAt(origin, target, up));

    auto sample_count = 1u;
    auto previous_width = 0, previous_height = 0;

    do
    {
        int width, height;
        window.GetFrameBufferSize(width, height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        shader.Bind();
        if (width != previous_width || height != previous_height)
        {
            previous_width = width;
            previous_height = height;

            sample_count = 1;

            glViewport(0, 0, width, height);

            glBindTexture(GL_TEXTURE_2D, accumulation_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindImageTexture(0, accumulation_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

            auto screen_to_camera = inverse(
                glm::perspectiveFov(
                    glm::radians(40.0f),
                    static_cast<float>(width),
                    static_cast<float>(height),
                    .3f,
                    100.f));

            shader.SetUniform(
                "Origin",
                [&origin](const GLint loc)
                {
                    glUniform3fv(loc, 1, &origin[0]);
                });
            shader.SetUniform(
                "CameraToWorld",
                [&camera_to_world](const GLint loc)
                {
                    glUniformMatrix4fv(loc, 1, GL_FALSE, &camera_to_world[0][0]);
                });
            shader.SetUniform(
                "ScreenToCamera",
                [&screen_to_camera](const GLint loc)
                {
                    glUniformMatrix4fv(loc, 1, GL_FALSE, &screen_to_camera[0][0]);
                });
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.SetUniform(
            "SampleCount",
            [&sample_count](const GLint loc)
            {
                glUniform1ui(loc, sample_count++);
            });

        vertex_array.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, INDICES);
        vertex_array.Unbind();
        shader.Unbind();

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::Begin("Stats"))
            ImGui::Text("Frame: %d", sample_count);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            window.MakeContextCurrent();
        }
    }
    while (window.Frame());

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}
