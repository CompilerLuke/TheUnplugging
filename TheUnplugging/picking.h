#pragma once
#include "id.h"
#include "shader.h"
#include "system.h"
#include "pass.h"
#include "handle.h"
#include "system.h"
#include "materialSystem.h"
#include "frameBuffer.h"

struct PickingPass : Pass {
	Uniform picking_shader_id_location;
	Handle<struct Shader> picking_shader;
	Framebuffer framebuffer;
	Handle<struct Texture> picking_map;
	
	void set_shader_params(Handle<Shader>, struct World&, struct RenderParams&) override;

	int pick(struct World&, struct UpdateParams&);
	void render(struct World&, struct RenderParams&) override;

	PickingPass(struct Window&);
};

struct PickingSystem : System {
	struct Editor& editor;
	Handle<struct Shader> outline_shader;
	Material outline_material;
	DrawCommandState object_state;
	DrawCommandState outline_state;

	PickingSystem(Editor&);

	void render(struct World&, struct RenderParams&) override;
};