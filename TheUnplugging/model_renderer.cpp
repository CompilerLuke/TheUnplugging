#include "model.h"
#include "transform.h"
#include "temporary.h"

REFLECT_STRUCT_BEGIN(ModelRenderer)
REFLECT_STRUCT_MEMBER(visible)
REFLECT_STRUCT_MEMBER(model_id)
REFLECT_STRUCT_MEMBER(materials)
REFLECT_STRUCT_END()

void ModelRendererSystem::render(World& world, RenderParams& params) {
	for (ID id : world.filter<ModelRenderer, Transform>(params.layermask)) {
		auto transform = world.by_id<Transform>(id);
		auto self = world.by_id<ModelRenderer>(id);

		if (!self->visible) continue;
		if (self->model_id == -1) continue;

		auto model = world.by_id<Model>(self->model_id);
		if (!model) continue;

		assert(model->materials.size() == self->materials.size());

		auto model_m = TEMPORARY_ALLOC(glm::mat4);
		*model_m = transform->compute_model_matrix();

		model->render(world.id_of(self), model_m, self->materials, params);
	}
}

void ModelRenderer::set_materials(World& world, std::vector<Material>& materials) {
	std::vector<Material> materials_in_order;

	if (this->model_id == -1) return;
	
	auto model = world.by_id<Model>(this->model_id);
	if (!model) return;

	for (auto& mat_name : model->materials) {
		auto mat = material_by_name(materials, mat_name);
		if (!mat) throw "Missing material";

		materials_in_order.push_back(*mat);
	}

	this->materials = materials_in_order;
}