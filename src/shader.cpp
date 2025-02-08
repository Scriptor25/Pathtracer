#include <fstream>
#include <iostream>
#include <pathtracer/shader.hpp>
#include <yaml-cpp/yaml.h>

struct StageInfo
{
    std::vector<std::string> Vertex;
    std::vector<std::string> Fragment;
};

StageInfo parseStageInfo(YAML::Node yaml)
{
    return StageInfo
    {
        .Vertex = yaml["vertex"].as<std::vector<std::string>>(),
        .Fragment = yaml["fragment"].as<std::vector<std::string>>(),
    };
}

struct ShaderInfo
{
    std::string ID;
    StageInfo Stages;
};

ShaderInfo parseShaderInfo(const std::filesystem::path& path)
{
    auto yaml = YAML::LoadFile(path.string());
    return ShaderInfo
    {
        .ID = yaml["id"].as<std::string>(),
        .Stages = parseStageInfo(yaml["stages"])
    };
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
            throw std::runtime_error("Failed to compile shader from " + path.string() + ":\n" + pMessage);
        }
    }

    glAttachShader(program, shader);
    glDeleteShader(shader);
}

pathtracer::Shader::Shader(const std::filesystem::path& path)
{
    m_Handle = glCreateProgram();

    const auto [ID, Stages] = parseShaderInfo(path);
    for (const auto& filename : Stages.Vertex)
        addShader(m_Handle, path.parent_path() / filename, GL_VERTEX_SHADER);
    for (const auto& filename : Stages.Fragment)
        addShader(m_Handle, path.parent_path() / filename, GL_FRAGMENT_SHADER);

    glLinkProgram(m_Handle);
    {
        GLint status;
        glGetProgramiv(m_Handle, GL_LINK_STATUS, &status);
        if (!status)
        {
            GLsizei length;
            GLchar pMessage[1000];
            glGetProgramInfoLog(m_Handle, 1000, &length, pMessage);
            throw std::runtime_error(std::string("Failed to link program:\n") + pMessage);
        }
    }
    glValidateProgram(m_Handle);
    {
        GLint status;
        glGetProgramiv(m_Handle, GL_VALIDATE_STATUS, &status);
        if (!status)
        {
            GLsizei length;
            GLchar pMessage[1000];
            glGetProgramInfoLog(m_Handle, 1000, &length, pMessage);
            throw std::runtime_error(std::string("Failed to validate program:\n") + pMessage);
        }
    }
}

pathtracer::Shader::~Shader()
{
    glDeleteProgram(m_Handle);
}

void pathtracer::Shader::Bind() const
{
    glUseProgram(m_Handle);
}

void pathtracer::Shader::Unbind() const
{
    glUseProgram(0);
}

void pathtracer::Shader::SetUniform(const std::string& name, const UniformCallback& callback) const
{
    callback(glGetUniformLocation(m_Handle, name.c_str()));
}
