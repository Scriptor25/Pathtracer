#include <fstream>
#include <iostream>
#include <pathtracer/shader.hpp>
#include <yaml-cpp/yaml.h>

using namespace std::string_view_literals;

struct StageInfo
{
    std::vector<std::string> Vertex;
    std::vector<std::string> Fragment;
};

static StageInfo parse_stage_info(YAML::Node yaml)
{
    return {
        .Vertex = yaml["vertex"].as<std::vector<std::string> >(),
        .Fragment = yaml["fragment"].as<std::vector<std::string> >(),
    };
}

struct ShaderInfo
{
    std::string ID;
    StageInfo Stages;
};

static ShaderInfo parse_shader_info(const std::filesystem::path &path)
{
    auto yaml = YAML::LoadFile(path.string());
    return {
        .ID = yaml["id"].as<std::string>(),
        .Stages = parse_stage_info(yaml["stages"])
    };
}

std::string load_shader_source(const std::filesystem::path &path, const bool is_recursive_call = false)
{
    constexpr auto include_keyword = "#include "sv;

    std::string source;
    std::ifstream stream(path);

    if (!stream)
    {
        std::cerr << "failed to open shader file " << path << std::endl;
        return source;
    }

    std::string line;
    while (std::getline(stream, line))
    {
        if (line.find(include_keyword) != std::string::npos)
        {
            line.erase(0, include_keyword.size());

            auto filename = line.substr(1, line.length() - 2); // Get rid of "" or <>
            source += load_shader_source(path.parent_path() / filename, true);

            continue;
        }

        source += line + '\n';
    }

    if (!is_recursive_call)
        source += '\0';

    stream.close();
    return source;
}

static void attach_shader(const GLuint program, const std::filesystem::path &path, const GLenum type)
{
    if (is_directory(path))
    {
        for (const auto &entry: std::filesystem::directory_iterator(path))
            attach_shader(program, entry.path(), type);
        return;
    }

    if (path.extension() != ".glsl")
        return;

    const auto source = load_shader_source(path);
    const auto source_ptr = source.c_str();

    const auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);
    {
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            GLsizei length;
            GLchar message[512];
            glGetShaderInfoLog(shader, 512, &length, message);
            throw std::runtime_error("failed to compile shader from " + path.string() + ":\n" + message);
        }
    }

    glAttachShader(program, shader);
    glDeleteShader(shader);
}

pathtracer::Shader::Shader(const std::filesystem::path &path)
{
    m_Handle = glCreateProgram();

    const auto [ID, Stages] = parse_shader_info(path);
    for (const auto &filename: Stages.Vertex)
        attach_shader(m_Handle, path.parent_path() / filename, GL_VERTEX_SHADER);
    for (const auto &filename: Stages.Fragment)
        attach_shader(m_Handle, path.parent_path() / filename, GL_FRAGMENT_SHADER);

    glLinkProgram(m_Handle);
    {
        GLint status;
        glGetProgramiv(m_Handle, GL_LINK_STATUS, &status);
        if (!status)
        {
            GLsizei length;
            GLchar message[512];
            glGetProgramInfoLog(m_Handle, 512, &length, message);
            throw std::runtime_error(std::string("Failed to link program:\n") + message);
        }
    }
    glValidateProgram(m_Handle);
    {
        GLint status;
        glGetProgramiv(m_Handle, GL_VALIDATE_STATUS, &status);
        if (!status)
        {
            GLsizei length;
            GLchar message[512];
            glGetProgramInfoLog(m_Handle, 512, &length, message);
            throw std::runtime_error(std::string("Failed to validate program:\n") + message);
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

void pathtracer::Shader::SetUniform(const std::string &name, const UniformConsumer &consumer) const
{
    consumer(glGetUniformLocation(m_Handle, name.c_str()));
}
