#define GLFW_INCLUDE_NONE

#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <vector>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <yaml-cpp/yaml.h>

void glfwErrorCallback(const int errorCode, const char* pDescription)
{
    std::cerr << "[GLFW 0x" << std::hex << errorCode << std::dec << "] " << pDescription << std::endl;
}

struct WindowState
{
    int xpos;
    int ypos;
    int width;
    int height;
};

void glfwToggleFullscreen(GLFWwindow* pWindow)
{
    if (glfwGetWindowMonitor(pWindow))
    {
        const auto pState = static_cast<WindowState*>(glfwGetWindowUserPointer(pWindow));
        glfwSetWindowMonitor(pWindow, nullptr, pState->xpos, pState->ypos, pState->width, pState->height, GLFW_DONT_CARE);
        delete pState;
    }
    else
    {
        const auto pState = new WindowState();
        glfwGetWindowPos(pWindow, &pState->xpos, &pState->ypos);
        glfwGetWindowSize(pWindow, &pState->width, &pState->height);
        glfwSetWindowUserPointer(pWindow, pState);

        const auto pMonitor = glfwGetPrimaryMonitor();
        const auto pMode = glfwGetVideoMode(pMonitor);
        glfwSetWindowMonitor(pWindow, pMonitor, 0, 0, pMode->width, pMode->height, pMode->refreshRate);
    }
}

void glfwKeyCallback(GLFWwindow* pWindow, const int key, const int /*scancode*/, const int action, const int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(pWindow, GLFW_TRUE);

    if (key == GLFW_KEY_F11 && action == GLFW_RELEASE)
        glfwToggleFullscreen(pWindow);
}

void glErrorCallback(const GLenum /*source*/, const GLenum /*type*/, const GLuint id, const GLenum severity, const GLsizei /*length*/, const GLchar* pMessage, const void* /*pUserParam*/)
{
    if (severity != GL_DEBUG_SEVERITY_HIGH)
        return;
    std::cerr << "[GL 0x" << std::hex << id << std::dec << "] " << pMessage << std::endl;
}

std::string loadShader(const std::filesystem::path& path, const bool isRecursiveCall = false)
{
    const std::string include_keyword = "#include ";

    std::string source;
    std::ifstream stream(path);

    if (!stream)
    {
        std::cerr << "Failed to open " << path << std::endl;
        return source;
    }

    std::string line;
    while (std::getline(stream, line))
    {
        if (line.find(include_keyword) != std::string::npos)
        {
            line.erase(0, include_keyword.size());

            std::string filename = line.substr(1, line.length() - 2); // Get rid of "" or <>
            source += loadShader(path.parent_path() / filename, true);

            continue;
        }

        source += line + '\n';
    }

    if (!isRecursiveCall)
        source += '\0';

    stream.close();
    return source;
}

void addShader(const GLuint program, const std::filesystem::path& path, const GLenum type)
{
    if (is_directory(path))
    {
        for (const auto& entry : std::filesystem::directory_iterator(path))
            addShader(program, entry.path(), type);
        return;
    }

    if (path.extension() != ".glsl")
        return;

    const auto source = loadShader(path);

    const auto pSource = source.c_str();

    const auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &pSource, nullptr);
    glCompileShader(shader);
    {
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            GLsizei length;
            GLchar pMessage[1000];
            glGetShaderInfoLog(shader, 1000, &length, pMessage);
            std::cerr << "In " << path << ":" << std::endl << pMessage << std::endl;
            return;
        }
    }

    glAttachShader(program, shader);
    glDeleteShader(shader);
}

struct StageInfo
{
    std::vector<std::string> Vertex;
    std::vector<std::string> Fragment;
};

struct ShaderInfo
{
    std::string ID;
    StageInfo Stages;
};

StageInfo parseStageInfo(YAML::Node yaml)
{
    return StageInfo
    {
        .Vertex = yaml["vertex"].as<std::vector<std::string>>(),
        .Fragment = yaml["fragment"].as<std::vector<std::string>>(),
    };
}

ShaderInfo parseShaderInfo(const std::filesystem::path& path)
{
    auto yaml = YAML::LoadFile(path.string());
    return ShaderInfo
    {
        .ID = yaml["id"].as<std::string>(),
        .Stages = parseStageInfo(yaml["stages"])
    };
}

int main(const int /*argc*/, const char** ppArgv)
{
    const auto assets = std::filesystem::path(ppArgv[0]).parent_path() / "assets";

    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
    {
        std::cerr << "glfwInit" << std::endl;
        return -1;
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
    const auto pWindow = glfwCreateWindow(800, 600, "Pathtracer", nullptr, nullptr);
    if (!pWindow)
    {
        std::cerr << "glfwCreateWindow" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(pWindow);
    glfwSetKeyCallback(pWindow, glfwKeyCallback);
    glfwSwapInterval(1);

    if (const auto error = glewInit())
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();

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
    ImGui_ImplGlfw_InitForOpenGL(pWindow, true);

    constexpr GLfloat pVertices[]{-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f};
    constexpr GLuint pIndices[]{0, 1, 2, 2, 3, 0};

    GLuint vao, vbo, accum;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pVertices), pVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &accum);

    const auto program = glCreateProgram();
    {
        const auto [ID, Stages] = parseShaderInfo(assets / "main.yaml");
        for (const auto& filename : Stages.Vertex)
            addShader(program, assets / filename, GL_VERTEX_SHADER);
        for (const auto& filename : Stages.Fragment)
            addShader(program, assets / filename, GL_FRAGMENT_SHADER);
    }
    glLinkProgram(program);
    {
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (!status)
        {
            std::cerr << "Failed to link program" << std::endl;

            GLsizei length;
            GLchar pMessage[1000];
            glGetProgramInfoLog(program, 1000, &length, pMessage);
            std::cerr << pMessage << std::endl;

            return -1;
        }
    }
    glValidateProgram(program);
    {
        GLint status;
        glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
        if (!status)
        {
            std::cerr << "Failed to validate program" << std::endl;

            GLsizei length;
            GLchar pMessage[1000];
            glGetProgramInfoLog(program, 1000, &length, pMessage);
            std::cerr << pMessage << std::endl;

            return -1;
        }
    }

    glm::vec3 origin(0.0f, 0.5f, -1.0f);
    glm::mat4 camera_to_world = inverse(lookAt(origin, origin + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 screen_to_camera;

    uint32_t sample_count = 1;

    int prev_width = 0, prev_height = 0;

    while (!glfwWindowShouldClose(pWindow))
    {
        int width, height;
        glfwGetFramebufferSize(pWindow, &width, &height);

        glUseProgram(program);
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

            screen_to_camera = inverse(glm::perspectiveFov(glm::radians(90.0f), static_cast<float>(width), static_cast<float>(height), 0.001f, 1000.0f));

            glUniform3fv(glGetUniformLocation(program, "Origin"), 1, &origin[0]);
            glUniformMatrix4fv(glGetUniformLocation(program, "CameraToWorld"), 1, GL_FALSE, &camera_to_world[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(program, "ScreenToCamera"), 1, GL_FALSE, &screen_to_camera[0][0]);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vao);
        glUniform1ui(glGetUniformLocation(program, "SampleCount"), sample_count++);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, pIndices);
        glBindVertexArray(0);
        glUseProgram(0);

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::Begin("Scene"))
        {
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(pWindow);
        }

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(pWindow);
    glfwTerminate();

    return 0;
}
