#include "VertexArray.hpp"

VertexArray::VertexArray()
{
	glCreateVertexArrays(1, &arrayHandle);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &arrayHandle);
}

void VertexArray::Bind()
{
	glBindVertexArray(arrayHandle);
}

void VertexArray::UnBind()
{
	glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(VertexBuffer&& buffer, std::initializer_list<GLVertexArrayAttribFormat> bufferLayout)
{
	GLuint vb_handle = buffer.GetHandle();
	// 각 속성(위치, 색상, 텍스처 좌표 등)에 대해 반복
	for (const GLVertexArrayAttribFormat& attribute : bufferLayout)
	{
		glEnableVertexArrayAttrib(arrayHandle, attribute.layoutLocation);
		glVertexArrayVertexBuffer(arrayHandle, attribute.layoutLocation, vb_handle, attribute.offset, attribute.stride);

		if (attribute.isIntegerType)
		{
			// 정수 타입 속성은 Attrib'I'Format 함수를 사용해야 함
			glVertexArrayAttribIFormat(arrayHandle, attribute.layoutLocation, attribute.dimension, attribute.type, attribute.relativeOffset);
		}
		else
		{
			// 실수 타입 속성은 기존 함수를 사용
			glVertexArrayAttribFormat(arrayHandle, attribute.layoutLocation, attribute.dimension, attribute.type, attribute.normalized, attribute.relativeOffset);
		}

		glVertexArrayAttribBinding(arrayHandle, attribute.layoutLocation, attribute.layoutLocation);
	}
	//VBO -> VAO
	vertexBuffers.push_back(std::move(buffer));
}

void VertexArray::AddIndexBuffer(IndexBuffer&& buffer)
{
	//인덱스의 개수와 버퍼를 저장후 VAO로 이동
	numIndices = buffer.GetCount();
	indexBuffer = std::move(buffer);
	glVertexArrayElementBuffer(arrayHandle, indexBuffer.GetIndicesHandle());
}