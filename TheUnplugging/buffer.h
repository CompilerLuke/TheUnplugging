#pragma once
#include <glad/glad.h>
#include <vector>

enum NumberType {
	Float, Int
};

GLenum numberType_to_gl(NumberType type);

struct VertexAttrib {
	unsigned int length;
	NumberType kind;
	unsigned int offset;
};

struct VertexBuffer {
	unsigned int vao = 0;
	unsigned int length = 0;

	void bind();
	
	VertexBuffer() {};

	template<typename T>
	VertexBuffer(std::vector<T>& vertices, std::vector<unsigned int>& indices, std::vector<VertexAttrib>& vertex_attribs) {
		unsigned int vao = 0;
		unsigned int vbo = 0;
		unsigned int ebo = 0;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_STATIC_DRAW);

		for (int i = 0; i < vertex_attribs.size(); i++) {
			VertexAttrib& va = vertex_attribs[i];

			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, va.length, numberType_to_gl(va.kind), false, sizeof(T), (void*)((std::size_t)va.offset));
		}

		glBindVertexArray(0);

		this->vao = vao;
		this->length = (unsigned int)indices.size();
	}
};