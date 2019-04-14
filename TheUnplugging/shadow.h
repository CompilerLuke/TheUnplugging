#pragma once

#include "renderPass.h"
#include "frameBuffer.h"

struct DepthMap {
	ID depth_shader;

	ID depth_map;
	Framebuffer depth_map_FBO;
	unsigned int width;
	unsigned int height;

	void set_shadow_params(struct Shader&, struct World&, struct RenderParams&);

	DepthMap(unsigned int, unsigned int, struct World&);
	void render_maps(struct World&, struct RenderParams&, glm::mat4 projection, glm::mat4 view);
};

struct ShadowPass : Pass {
	ID shadow_mask_shader;
	ID screenspace_blur_shader;

	ID depth_prepass;
	DepthMap deffered_map_cascade;
	ShadowMask shadow_mask;
	ShadowMask ping_pong_shadow_mask;

	void render(struct World&, struct RenderParams&) override;
	void set_shader_params(struct Shader&, struct World&, struct RenderParams&) override;
	void set_shadow_params(struct Shader&, struct World&, struct RenderParams&);

	ShadowPass(struct Window& window, struct World&, ID depth_prepass);
};

struct ShadowMask {
	ID shadow_mask_map;
	Framebuffer shadow_mask_map_fbo;

	void set_shadow_params(struct Shader&, struct World&, struct RenderParams&);

	ShadowMask(struct Window& window, struct World&);
};