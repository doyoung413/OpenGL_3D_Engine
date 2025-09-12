#define STB_IMAGE_IMPLEMENTATION

#include "Texture.hpp"
#include "stb_image.h"
#include <iostream>

Texture::Texture(const std::string& path)
    : textureHandle(0), filePath(path), size(0 , 0), bpp(0)
{
    int width, height;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* localBuffer = stbi_load(path.c_str(), &width, &height, &bpp, 4);

    if (localBuffer)
    {
        size = glm::vec2(width, height);

        // 텍스처 객체를 생성
        glGenTextures(1, &textureHandle);
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        // 텍스처 필터링 및 래핑 모드를 설정
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 로드한 데이터를 GPU의 텍스처 객체 메모리로 전송
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
        // 밉맵을 생성
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(localBuffer);
    }
    else
    {
        std::cout << "Fail to load: " << path << std::endl;
    }
}

Texture::~Texture()
{
    glDeleteTextures(1, &textureHandle);
}

Texture::Texture(Texture&& other) noexcept
    : textureHandle(other.textureHandle), filePath(std::move(other.filePath)),
    size(other.size), bpp(other.bpp)
{
    other.textureHandle = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        glDeleteTextures(1, &textureHandle);
        textureHandle = other.textureHandle;
        filePath = std::move(other.filePath);
        size.x = other.size.x;
        size.y = other.size.y;
        bpp = other.bpp;
        other.textureHandle = 0;
    }
    return *this;
}

void Texture::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}