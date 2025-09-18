#include "IndexBuffer.hpp"
#include <glew.h>

IndexBuffer::IndexBuffer(std::span<const unsigned int> indices)
	: count(static_cast<int>(indices.size()))
{
	glCreateBuffers(1, &indicesHandle);
	glNamedBufferStorage(indicesHandle, indices.size_bytes(), indices.data(), GL_DYNAMIC_STORAGE_BIT);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &indicesHandle);
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : indicesHandle(other.indicesHandle), count(other.count)
{
    other.indicesHandle = 0;
    other.count = 0;
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
    if (this != &other)
    {
        glDeleteBuffers(1, &indicesHandle);
        indicesHandle = other.indicesHandle;
        count = other.count;
        other.indicesHandle = 0;
        other.count = 0;
    }
    return *this;
}

void IndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesHandle);
}

void IndexBuffer::UnBind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}