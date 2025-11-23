#define STB_IMAGE_IMPLEMENTATION

#include "Texture.hpp"
#include "stb_image.h"
#include <iostream>

Texture::Texture(const std::string& path)
    : textureHandle(0), filePath(path), size(0, 0), bpp(0)
{
    stbi_set_flip_vertically_on_load(1); // .hdr, .png 모두 뒤집어야 함

    // 파일 확장자 확인
    std::string extension = path.substr(path.find_last_of("."));
    bool isHDR = (extension == ".hdr");

    unsigned char* localBuffer = nullptr;
    float* hdrBuffer = nullptr;

    if (isHDR)
    {
        // HDR 텍스처 로드 (32비트 float)
        hdrBuffer = stbi_loadf(path.c_str(), &size.x, &size.y, &bpp, 0);
        if (hdrBuffer)
        {
            internalFormat = GL_RGB16F; // 16비트 float (32F도 가능)
            dataFormat = GL_RGB;
        }
    }
    else
    {
        // LDR(일반) 텍스처 로드 (8비트 unsigned char)
        localBuffer = stbi_load(path.c_str(), &size.x, &size.y, &bpp, 4); // RGBA 강제
        if (localBuffer)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
    }

    if (localBuffer || hdrBuffer)
    {
        glGenTextures(1, &textureHandle);
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        // 텍스처 필터링 및 래핑 모드 설정
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // IBL은 CLAMP_TO_EDGE 권장
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (isHDR)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0, dataFormat, GL_FLOAT, hdrBuffer);
            stbi_image_free(hdrBuffer);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0, dataFormat, GL_UNSIGNED_BYTE, localBuffer);
            stbi_image_free(localBuffer);
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        std::cout << "Fail to load texture: " << path << std::endl;
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