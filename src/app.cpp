#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <assimp/postprocess.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include <pathtracer/app.hpp>
#include <pathtracer/window.hpp>

glm::mat4 pathtracer::App::CAMERA_TO_WORLD = inverse(lookAt(ORIGIN, TARGET, UP));

static void gl_debug_message_callback(
    const GLenum source,
    const GLenum type,
    const GLuint id,
    const GLenum severity,
    const GLsizei length,
    const GLchar *message,
    const void *user_param)
{
    if (severity != GL_DEBUG_SEVERITY_HIGH)
        return;

    std::cerr << "[GL 0x" << std::hex << id << std::dec << "] " << message << std::endl;
}

pathtracer::App::App()
{
    m_Assets = std::filesystem::canonical("assets");
    m_Window = std::make_unique<Window>(600, 600, "PathTracer", m_Assets / "icon.png");

    if (const auto error = glewInit())
        throw std::runtime_error(
            "[GLEW 0x"
            + std::to_string(error)
            + "] failed to initialize glew: "
            + reinterpret_cast<const char *>(glewGetErrorString(error)));

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
    ImGui_ImplGlfw_InitForOpenGL(m_Window->Handle(), true);

    m_Window->SetFramebufferSizeCallback(
        [this]
        {
            OnFrame();
            (void) m_Window->Spin();
        });

    OnStart();
    do
        OnFrame();
    while (m_Window->Spin());
}

pathtracer::App::~App()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

void pathtracer::App::OnStart()
{
    m_VertexArray = std::make_unique<VertexArray>();
    m_VertexBuffer = std::make_unique<Buffer>(GL_ARRAY_BUFFER, GL_STATIC_DRAW);

    m_VertexArray->Bind();
    m_VertexBuffer->Bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    m_VertexBuffer->Data(sizeof(VERTICES), VERTICES);
    m_VertexBuffer->Unbind();
    m_VertexArray->Unbind();

    glGenTextures(1, &m_AccumulationTexture);

    m_Shader = std::make_unique<Shader>(m_Assets / "main.yaml");

    m_Scene = std::make_unique<Scene>();
    m_Scene->LoadModel(m_Assets / "objects" / "cornell_box.obj", aiProcess_GenNormals);

    m_Scene->LoadModel(m_Assets / "objects" / "cow.obj", aiProcess_GenSmoothNormals);
    m_Scene->GetLastModel().Transform = scale(
        rotate(
            translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.65f, 0.0f)),
            glm::radians(-135.0f),
            normalize(glm::vec3(0.0f, 1.0f, 0.0f))),
        glm::vec3(0.1f));
    m_Scene->GetLastModel().InverseTransform = inverse(m_Scene->GetLastModel().Transform);
    m_Scene->GetLastModel().NormalTransform = transpose(m_Scene->GetLastModel().InverseTransform);

    m_Scene->LoadModel(m_Assets / "objects" / "teapot.obj", aiProcess_GenSmoothNormals);
    m_Scene->GetLastModel().Transform = scale(
        rotate(
            translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.0f, 0.0f)),
            glm::radians(-45.0f),
            normalize(glm::vec3(0.0f, 1.0f, 0.0f))),
        glm::vec3(0.2f));
    m_Scene->GetLastModel().InverseTransform = inverse(m_Scene->GetLastModel().Transform);
    m_Scene->GetLastModel().NormalTransform = transpose(m_Scene->GetLastModel().InverseTransform);

    m_Scene->Upload();
}

void pathtracer::App::OnFrame()
{
    int width, height;
    m_Window->GetFramebufferSize(width, height);

    if (width == 0 || height == 0)
    {
        glfwWaitEvents();
        return;
    }

    m_Shader->Bind();
    if (width != m_PreviousWidth || height != m_PreviousHeight)
    {
        m_SampleCount = 1u;

        m_PreviousWidth = width;
        m_PreviousHeight = height;

        glViewport(0, 0, width, height);

        glBindTexture(GL_TEXTURE_2D, m_AccumulationTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindImageTexture(0, m_AccumulationTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        auto screen_to_camera = inverse(
            glm::perspectiveFov(
                glm::radians(40.0f),
                static_cast<float>(width),
                static_cast<float>(height),
                .3f,
                100.f));

        m_Shader->SetUniform(
            "Origin",
            [](const GLint loc)
            {
                glUniform3fv(loc, 1, &ORIGIN[0]);
            });
        m_Shader->SetUniform(
            "CameraToWorld",
            [](const GLint loc)
            {
                glUniformMatrix4fv(loc, 1, GL_FALSE, &CAMERA_TO_WORLD[0][0]);
            });
        m_Shader->SetUniform(
            "ScreenToCamera",
            [&screen_to_camera](const GLint loc)
            {
                glUniformMatrix4fv(loc, 1, GL_FALSE, &screen_to_camera[0][0]);
            });
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_Shader->SetUniform(
        "SampleCount",
        [this](const GLint loc)
        {
            glUniform1ui(loc, m_SampleCount);
        });

    m_VertexArray->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, INDICES);
    m_VertexArray->Unbind();
    m_Shader->Unbind();

    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::Begin("Stats"))
        ImGui::Text("Frame: %d", m_SampleCount);
    ImGui::End();

    m_SampleCount++;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        m_Window->MakeContextCurrent();
    }
}
