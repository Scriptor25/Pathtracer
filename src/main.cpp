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
#include <yaml-cpp/yaml.h>

void glErrorCallback(const GLenum /*source*/, const GLenum /*type*/, const GLuint id, const GLenum severity, const GLsizei /*length*/, const GLchar* pMessage, const void* /*pUserParam*/)
{
    if (severity != GL_DEBUG_SEVERITY_HIGH)
        return;
    std::cerr << "[GL 0x" << std::hex << id << std::dec << "] " << pMessage << std::endl;
}

int main(const int /*argc*/, const char** ppArgv)
{
    const auto assets = std::filesystem::path(ppArgv[0]).parent_path() / "assets";

    const pathtracer::Window window(600, 600);

    if (const auto error = glewInit())
    {
        std::cerr << "glewInit " << glewGetErrorString(error) << std::endl;
        return -1;
    }

    glDebugMessageCallback(glErrorCallback, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glClearColor(0.2f, 0.3f, 1.0f, 1.0f);

    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigDockingTransparentPayload = true;
    io.ConfigViewportsNoTaskBarIcon = true;

    ImGui_ImplOpenGL3_Init();
    ImGui_ImplGlfw_InitForOpenGL(window.GLFW(), true);

    constexpr GLfloat pVertices[]{-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f};
    constexpr GLuint pIndices[]{0, 1, 2, 2, 3, 0};

    const pathtracer::VertexArray vertex_array;
    vertex_array.Bind();
    const pathtracer::Buffer vertex_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    vertex_buffer.Bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    vertex_buffer.Data(sizeof(pVertices), pVertices);
    vertex_buffer.Unbind();
    vertex_array.Unbind();

    GLuint accum;
    glGenTextures(1, &accum);

    pathtracer::Shader* pShader;
    try { pShader = new pathtracer::Shader(assets / "main.yaml"); }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << std::endl;
        return -1;
    }

    pathtracer::Scene scene;
    scene.LoadModel(assets / "objects" / "cornell_box.obj", aiProcess_GenNormals);

    // scene.LoadModel(assets / "objects" / "axis.obj");
    // scene.GetLastModel().Transform = scale(rotate(translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.95f, 0.0f)), glm::radians(45.0f), normalize(glm::vec3(0.0f, 1.0f, 0.0f))), glm::vec3(0.1f));
    // scene.GetLastModel().InverseTransform = inverse(scene.GetLastModel().Transform);
    // scene.GetLastModel().NormalTransform = transpose(scene.GetLastModel().InverseTransform);

    scene.LoadModel(assets / "objects" / "cow.obj", aiProcess_GenSmoothNormals);
    scene.GetLastModel().Transform = scale(rotate(translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.65f, 0.0f)), glm::radians(-135.0f), normalize(glm::vec3(0.0f, 1.0f, 0.0f))), glm::vec3(0.1f));
    scene.GetLastModel().InverseTransform = inverse(scene.GetLastModel().Transform);
    scene.GetLastModel().NormalTransform = transpose(scene.GetLastModel().InverseTransform);

    scene.LoadModel(assets / "objects" / "teapot.obj", aiProcess_GenSmoothNormals);
    scene.GetLastModel().Transform = scale(rotate(translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.0f, 0.0f)), glm::radians(-45.0f), normalize(glm::vec3(0.0f, 1.0f, 0.0f))), glm::vec3(0.2f));
    scene.GetLastModel().InverseTransform = inverse(scene.GetLastModel().Transform);
    scene.GetLastModel().NormalTransform = transpose(scene.GetLastModel().InverseTransform);

    scene.Upload();

    glm::vec3 origin(0.0f, 0.0f, 3.75f);
    glm::mat4 camera_to_world = inverse(lookAt(origin, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

    unsigned sample_count = 1;
    int prev_width = 0, prev_height = 0;

    do
    {
        int width, height;
        window.GetFramebufferSize(&width, &height);

        if (width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        pShader->Bind();
        if (width != prev_width || height != prev_height)
        {
            prev_width = width;
            prev_height = height;

            sample_count = 1;

            glViewport(0, 0, width, height);

            glBindTexture(GL_TEXTURE_2D, accum);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindImageTexture(0, accum, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

            glm::mat4 screen_to_camera = inverse(glm::perspectiveFov(glm::radians(40.0f), static_cast<float>(width), static_cast<float>(height), 0.001f, 1000.0f));

            pShader->SetUniform("Origin", [&origin](const GLint loc) { glUniform3fv(loc, 1, &origin[0]); });
            pShader->SetUniform("CameraToWorld", [&camera_to_world](const GLint loc) { glUniformMatrix4fv(loc, 1, GL_FALSE, &camera_to_world[0][0]); });
            pShader->SetUniform("ScreenToCamera", [&screen_to_camera](const GLint loc) { glUniformMatrix4fv(loc, 1, GL_FALSE, &screen_to_camera[0][0]); });
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pShader->SetUniform("SampleCount", [&sample_count](const GLint loc) { glUniform1ui(loc, sample_count++); });

        vertex_array.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, pIndices);
        vertex_array.Unbind();
        pShader->Unbind();

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::Begin("Stats"))
        {
            ImGui::Text("Frame: %d", sample_count);
        }
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

    return 0;
}
