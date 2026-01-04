#pragma once
#include <span>
#include <glew.h>
class VertexBuffer
{
public:
	VertexBuffer() = default;
	template <typename T, auto N = std::dynamic_extent>
	VertexBuffer(std::span<T, N> data, size_t size_)
	{
		glCreateBuffers(1, &bufferHandle);
		glNamedBufferStorage(bufferHandle, size_, nullptr, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferSubData(bufferHandle, 0, size_, data.data());  // ก็ Now correctly uses size_!
		size = size_;
	}
	~VertexBuffer();

	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;

	VertexBuffer(VertexBuffer&& other) noexcept;
	VertexBuffer& operator=(VertexBuffer&& other) noexcept;

	void Bind();
	void UnBind();

	template <typename T, auto N = std::dynamic_extent>
	void SetData(std::span<T, N> data, size_t offset = 0) const
	{
		glNamedBufferSubData(bufferHandle, offset, static_cast<size_t>(data.size_bytes()), data.data());
	}

	unsigned int GetHandle() const noexcept { return bufferHandle; }
	size_t GetSizeBytes() const noexcept { return size; }

private:
	unsigned int bufferHandle = 0;
	size_t  size = 0;
};