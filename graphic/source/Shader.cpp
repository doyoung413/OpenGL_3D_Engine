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
    
    if (vertexSource.empty())
    {
        std::cerr << "버텍스 셰이더 파일을 읽지 못했습니다!" << std::endl;
        return;
    }
    else if (fragmentSource.empty())
    {
        std::cerr << "프레그먼트 셰이더 파일을 읽지 못했습니다!" << std::endl;
        return;
    }

    rendererID = CreateProgram(vertexSource, fragmentSource);
    
    if (rendererID == 0)
    {
        std::cerr << "셰이더 프로그램 생성에 실패했습니다!" << std::endl;
        return;
    }

    // 셰이더 리플렉션: 모든 활성 유니폼 변수의 목록을 가져와 저장
    GLint numActiveUniforms = 0;
    glGetProgramiv(rendererID, GL_ACTIVE_UNIFORMS, &numActiveUniforms);

    GLchar uniformName[256];
    for (GLint i = 0; i < numActiveUniforms; i++)
    {
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(rendererID, i, sizeof(uniformName), &length, &size, &type, uniformName);
        activeUniforms.insert(uniformName);
    }
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

bool Shader::HasUniform(const std::string& name) const
{
    if (activeUniforms.count(name)) {
        return true;
    }

    if (activeUniforms.count(name + "[0]")) {
        return true;
    }

    return false;
}

std::string Shader::ReadFile(const std::string& filepath)
{
    std::ifstream stream(filepath);
    if (!stream.is_open()) {
        std::cerr << "셰이더 파일을 열 수 없습니다: " << filepath << std::endl;
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
        if (length > 0)
        {
            std::vector<char> message(length);
            glGetShaderInfoLog(id, length, nullptr, &message[0]);
            std::cerr << (type == GL_VERTEX_SHADER ? "버텍스" : "프래그먼트") << " 셰이더 컴파일 실패!" << std::endl;
            std::cerr << message.data() << std::endl;
        }
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

    if (vs == 0 || fs == 0)
    {
        if (vs == 0)
        {
            std::cerr << "버텍스 셰이더 컴파일을 실패했습니다!" << std::endl;
        }
        if (fs == 0)
        {
            std::cerr << "프래그먼트 셰이더 컴파일을 실패했습니다!" << std::endl;
        }

        glDeleteProgram(program);
        if (vs != 0) 
        {
            glDeleteShader(vs);
        }
        if (fs != 0) 
        {
            glDeleteShader(fs);
        }
        return 0;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length > 0)
        {
            std::vector<char> message(length);
            glGetProgramInfoLog(program, length, nullptr, &message[0]);
            std::cerr << "셰이더 프로그램 링크에 실패했습니다!" << std::endl;
            std::cerr << message.data() << std::endl;
        }
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glValidateProgram(program);

    int validateStatus;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
    if (validateStatus == GL_FALSE)
    {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        if (length > 0)
        {
            std::vector<char> message(length);
            glGetProgramInfoLog(program, length, nullptr, &message[0]);
            std::cerr << "셰이더 프로그램 유효성 검사를 실패했습니다!" << std::endl;
            std::cerr << message.data() << std::endl;
        }
    }

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
    {
        std::cout << "Uniform '" << name << "'을(를) 찾을 수 없습니다." << std::endl;
    }

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

void Shader::SetUniformMat4fv(const std::string& name, int count, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, glm::value_ptr(matrix));
}
