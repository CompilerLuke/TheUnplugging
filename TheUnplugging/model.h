#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "buffer.h"
#include "culling.h"
#include "materialSystem.h"
#include "ecs.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coord;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct Mesh {
	VertexBuffer buffer;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	AABB aabb;
	unsigned int material_id;

	Mesh() {};
	void submit();
	void render(ID, glm::mat4&, std::vector<Material>&);
};

struct Model {
	std::string path;
	std::vector<Mesh> meshes;
	std::vector<std::string> materials;
};

struct ModelRenderer {
	int model_id;
	std::vector<Material> materials;
};