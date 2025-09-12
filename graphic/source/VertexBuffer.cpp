#include "VertexBuffer.hpp"

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &bufferHandle);
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : bufferHandle(other.bufferHandle), size(other.size)
{
    other.bufferHandle = 0;
    other.size = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (this != &other)
    {
        glDeleteBuffers(1, &bufferHandle);
        bufferHandle = other.bufferHandle;
        size = other.size;
        other.bufferHandle = 0;
        other.size = 0;
    }
    return *this;
}

void VertexBuffer::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, bufferHandle);
}

void VertexBuffer::UnBind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}