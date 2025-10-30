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
	// �� �Ӽ�(��ġ, ����, �ؽ�ó ��ǥ ��)�� ���� �ݺ�
	for (const GLVertexArrayAttribFormat& attribute : bufferLayout)
	{
		glEnableVertexArrayAttrib(arrayHandle, attribute.layoutLocation);
		glVertexArrayVertexBuffer(arrayHandle, attribute.layoutLocation, vb_handle, attribute.offset, attribute.stride);

		if (attribute.isIntegerType)
		{
			// ���� Ÿ�� �Ӽ��� Attrib'I'Format �Լ��� ����ؾ� ��
			glVertexArrayAttribIFormat(arrayHandle, attribute.layoutLocation, attribute.dimension, attribute.type, attribute.relativeOffset);
		}
		else
		{
			// �Ǽ� Ÿ�� �Ӽ��� ���� �Լ��� ���
			glVertexArrayAttribFormat(arrayHandle, attribute.layoutLocation, attribute.dimension, attribute.type, attribute.normalized, attribute.relativeOffset);
		}

		glVertexArrayAttribBinding(arrayHandle, attribute.layoutLocation, attribute.layoutLocation);
	}
	//VBO -> VAO
	vertexBuffers.push_back(std::move(buffer));
}

void VertexArray::AddIndexBuffer(IndexBuffer&& buffer)
{
	//�ε����� ������ ���۸� ������ VAO�� �̵�
	numIndices = buffer.GetCount();
	indexBuffer = std::move(buffer);
	glVertexArrayElementBuffer(arrayHandle, indexBuffer.GetIndicesHandle());
}