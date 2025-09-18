#pragma once
#include <string>
#include <unordered_map>
#include <glm.hpp>

class Shader
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformVec3(const std::string& name, const glm::vec3& vector);
    void SetUniformVec4(const std::string& name, const glm::vec4& vector);
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

    int GetShaderID() { return rendererID; }
private:
    std::string ReadFile(const std::string& filepath);
    unsigned int CompileShader(unsigned int type, const std::string& source);
    unsigned int CreateProgram(const std::string& vertexShader, const std::string& fragmentShader);
    int GetUniformLocation(const std::string& name);

    unsigned int rendererID;
    std::unordered_map<std::string, int> uniformLocationCache;
};
