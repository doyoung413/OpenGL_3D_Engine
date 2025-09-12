#pragma once
#include <span>

class IndexBuffer
{
public:
	IndexBuffer() = default;
	IndexBuffer(std::span<const unsigned int> indices);
	~IndexBuffer();

	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer& operator=(const IndexBuffer&) = delete;

	IndexBuffer(IndexBuffer&& other) noexcept;
	IndexBuffer& operator=(IndexBuffer&& other) noexcept;

	void Bind();
	void UnBind();

	unsigned int GetCount() const { return count; }
	unsigned int GetIndicesHandle() const { return indicesHandle; }
private:
	unsigned int indicesHandle = 0;
	unsigned int count = 0;
};