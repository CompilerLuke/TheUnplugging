#pragma once

#include "id.h"
#include "frameBuffer.h"

struct FogMap {
	Framebuffer fbo;
	ID map;

	FogMap(struct World&, unsigned int, unsigned int);
};

struct ShadowParams {
	ID depth_map = 0;
	glm::vec2 in_range;
	glm::mat4 to_light;
	glm::mat4 to_world;
	unsigned int cascade = 0;

	ShadowParams() {};
};

struct VolumetricPass {
	ID depth_prepass;
	FogMap calc_fog;

	ID volume_shader;
	ID upsample_shader;

	VolumetricPass(struct World&, struct Window&, ID);
	void clear();

	void render_with_cascade(struct World&, struct RenderParams&, ShadowParams& shadow_params);
	void render_upsampled(struct World&, ID);
};