#include "model.h"
#include "ecs.h"
#include "temporary.h"

void Mesh::submit() {
	std::vector<VertexAttrib> attribs = {
		{3, Float, offsetof(Vertex, position)},
		{3, Float, offsetof(Vertex, normal)},
		{2, Float, offsetof(Vertex, tex_coord)},
		{3, Float, offsetof(Vertex, tangent)},
		{3, Float, offsetof(Vertex, bitangent)}
	};

	for (auto &v : vertices) {
		this->aabb.update(v.position);
	}

	this->buffer = VertexBuffer(vertices, indices, attribs);
}

void Mesh::render(ID id, glm::mat4& model, std::vector<Material>& materials) {
	auto material = &materials[material_id];
	auto aabb = alloc_temporary<AABB>();
	*aabb = this->aabb.apply(model);
}