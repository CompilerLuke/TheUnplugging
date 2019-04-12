#include "model.h"

void Model::on_load(World& world) {
	for (auto& mesh : meshes) {
		mesh.submit();
	}
}

void Model::render(ID id, glm::mat4* model, std::vector<Material>& materials, RenderParams& params) {
	for (auto& mesh : meshes) {
		mesh.render(id, model, materials, params);
	}
}

