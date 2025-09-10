#include "VertexBuffer.hpp"

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &bufferHandle);
}

void VertexBuffer::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, bufferHandle);
}

void VertexBuffer::UnBind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}