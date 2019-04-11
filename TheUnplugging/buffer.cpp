#include "buffer.h"

void VertexBuffer::bind() {
	glBindVertexArray(vao);
}

GLenum numberType_to_gl(NumberType type) {
	if (type == Float) return GL_FLOAT;
	else return GL_INT;
}