#pragma once
#define TEXTURE_H

#include <string>
#include <glew.h>
#include "glm.hpp"

class Texture
{
public:
    Texture(const std::string& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    int GetWidth() const { return size.x; }
    int GetHeight() const { return size.y; }

private:
    GLuint textureHandle = 0;
    std::string filePath;
    glm::ivec2 size{ 0,0 };
    int bpp = 0; // BPP: Bits Per Pixel

    GLenum internalFormat = GL_RGBA8;
    GLenum dataFormat = GL_RGBA;
};
