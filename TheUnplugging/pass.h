#pragma once

struct Pass {
	virtual void render(struct World&, struct RenderParams&) {}
	virtual void set_shader_params(struct Shader&, struct World&, struct RenderParams&) {};
};