#pragma once
#include "IndexBuffer.hpp"
#include "VertexBuffer.hpp"
#include <memory>
#include <vector>

struct GLVertexArrayAttribFormat
{
	GLint dimension = 1;
	GLuint layoutLocation = 0;
	GLboolean normalized = GL_FALSE;
	GLuint relativeOffset = 0;
	GLenum type = GL_FLOAT;
	GLintptr offset = 0;
	GLsizei stride = 0;
};

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void Bind();
	void UnBind();

	void AddVertexBuffer(VertexBuffer&& buffer, std::initializer_list<GLVertexArrayAttribFormat> bufferLayout);
	void AddIndexBuffer(IndexBuffer&& buffer);

	GLuint GetHandle() const noexcept { return arrayHandle; }
	std::vector<VertexBuffer>& GetVertexBuffers() { return vertexBuffers; }
	IndexBuffer& GetIndexBuffer() { return indexBuffer; }
	GLsizei GetIndicesCount() const noexcept { return numIndices; }
	GLsizei GetVertexCount() const { return numVertices; }
	
	void SetVertexCount(int count) { numVertices = count; }
private:
	std::vector<VertexBuffer> vertexBuffers;
	IndexBuffer               indexBuffer;
	GLsizei                   numVertices = 0;
	GLsizei                   numIndices = 0;
	GLuint					  arrayHandle = 0;
};