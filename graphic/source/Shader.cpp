#include "Shader.hpp"
#include <glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <gtc/type_ptr.hpp>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    : rendererID(0)
{
    std::string vertexSource = ReadFile(vertexPath);
    std::string fragmentSource = ReadFile(fragmentPath);
    rendererID = CreateProgram(vertexSource, fragmentSource);
}

Shader::~Shader()
{
    glDeleteProgram(rendererID);
}

Shader::Shader(Shader&& other) noexcept
    : rendererID(other.rendererID), uniformLocationCache(std::move(other.uniformLocationCache))
{
    other.rendererID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        glDeleteProgram(rendererID);
        rendererID = other.rendererID;
        uniformLocationCache = std::move(other.uniformLocationCache);
        other.rendererID = 0;
    }
    return *this;
}

std::string Shader::ReadFile(const std::string& filepath)
{
    std::ifstream stream(filepath);
    if (!stream.is_open()) {
        std::cerr << "���̴� ������ �� �� �����ϴ�: " << filepath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << stream.rdbuf();
    return ss.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << (type == GL_VERTEX_SHADER ? "���ؽ�" : "�����׸�Ʈ") << " ���̴� ������ ����!" << std::endl;
        std::cerr << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

unsigned int Shader::CreateProgram(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::Bind() const
{
    glUseProgram(rendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (uniformLocationCache.find(name) != uniformLocationCache.end())
        return uniformLocationCache[name];

    int location = glGetUniformLocation(rendererID, name.c_str());
    if (location == -1)
        std::cout << "���: Uniform '" << name << "'��(��) ã�� �� �����ϴ�." << std::endl;

    uniformLocationCache[name] = location;
    return location;
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformVec3(const std::string& name, const glm::vec3& vector)
{
    glUniform3fv(GetUniformLocation(name), 1, &vector[0]);
}

void Shader::SetUniformVec4(const std::string& name, const glm::vec4& vector)
{
    glUniform4fv(GetUniformLocation(name), 1, &vector[0]);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}
