#pragma once
#include "device.h"
#include "id.h"
#include "frameBuffer.h"

struct Pass {
	virtual void render(struct World&, struct RenderParams&) {}
	virtual void set_shader_params(struct Shader&, struct World&, struct RenderParams&) {};
};

struct MainPass : Pass {
	Device device;

	ID frame_map;
	Framebuffer current_frame;
	
	void render(struct World&, struct RenderParams&) override;
	void set_shader_params(struct Shader&, struct World&, struct RenderParams&) override;

	MainPass(struct World&, struct Window&);
};
