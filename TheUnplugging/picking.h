#pragma once
#include "id.h"
#include "shader.h"
#include "system.h"
#include "pass.h"

struct PickingPass : Pass {
	unsigned int fbo;
	unsigned int width;
	unsigned int height;
	ID picking_shader;
	ID outline_shader;
	Uniform picking_shader_id_location;

	void set_shader_params(struct Shader&, struct World&, struct RenderParams&) override;

	int pick(struct World&, struct UpdateParams&);
	void render(struct World&, struct RenderParams&) override;
};