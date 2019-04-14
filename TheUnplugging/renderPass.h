#pragma once
#include "device.h"
#include "id.h"
#include "frameBuffer.h"
#include "shadow.h"
#include "pass.h"

struct MainPass : Pass {
	Device device;
	
	DepthMap depth_prepass;

	ShadowPass shadow_pass;

	ID frame_map;
	Framebuffer current_frame;
	
	void render(struct World&, struct RenderParams&) override;
	void set_shader_params(struct Shader&, struct World&, struct RenderParams&) override;

	MainPass(struct World&, struct Window&);
};
