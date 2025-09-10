#include "IndexBuffer.hpp"
#include <glew.h>

IndexBuffer::IndexBuffer(std::span<const unsigned int> indices)
	: count(indices.size())
{
	glCreateBuffers(1, &indicesHandle);
	glNamedBufferStorage(indicesHandle, indices.size_bytes(), indices.data(), GL_DYNAMIC_STORAGE_BIT);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &indicesHandle);
}

void IndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesHandle);
}

void IndexBuffer::UnBind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}