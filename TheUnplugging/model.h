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
#include "handle.h"

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
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	AABB aabb;
	unsigned int material_id;

	Mesh() {};
	void submit();
	void render(ID, glm::mat4*, vector<Material>&, RenderParams&);

	REFLECT()
};

struct Model {
	std::string path;
	vector<Mesh> meshes;
	vector<std::string> materials;

	void on_load();
	void load_in_place();
	void render(ID, glm::mat4*, vector<Material>&, RenderParams&);

	REFLECT()
};

Handle<Model> load_Model(World&, const std::string&);

struct ModelRenderer {
	bool visible = true;
	Handle<Model> model_id;

	void set_materials(World&, vector<Material>& materials);

	REFLECT()
};

struct ModelRendererSystem : System {
	void render(World&, RenderParams&) override;
};