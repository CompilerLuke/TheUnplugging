#include "model.h"
#include "ecs.h"
#include "temporary.h"
#include "draw.h"

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

	new (&this->buffer) VertexBuffer(vertices, indices, attribs);
}

void Mesh::render(ID id, glm::mat4* model, std::vector<Material>& materials, RenderParams& params) {
	auto material = &materials[material_id];
	auto aabb = TEMPORARY_ALLOC(AABB);
	*aabb = this->aabb.apply(*model);

	DrawCommand cmd(id, model, aabb, &buffer, material);
	params.command_buffer.submit(cmd);
}
