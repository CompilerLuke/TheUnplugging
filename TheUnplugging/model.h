#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "buffer.h"
#include "culling.h"
#include "materialSystem.h"
#include "ecs.h"
#include "reflection.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coord;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	REFLECT()
};

struct Mesh {
	VertexBuffer buffer;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	AABB aabb;
	unsigned int material_id;

	Mesh() {};
	void submit();
	void render(ID, glm::mat4*, std::vector<Material>&, RenderParams&);

	REFLECT()
};

struct Model {
	std::string path;
	std::vector<Mesh> meshes;
	std::vector<std::string> materials;

	void on_load(World&);
	void load_in_place(World&);
	void render(ID, glm::mat4*, std::vector<Material>&, RenderParams&);

	REFLECT()
};

Model* load_Model(World&, const std::string&);

struct ModelRenderer {
	bool visible = true;
	int model_id = -1;
	std::vector<Material> materials;

	void set_materials(World&, std::vector<Material>& materials);

	REFLECT()
};

struct ModelRendererSystem : System {
	void render(World&, RenderParams&) override;
};