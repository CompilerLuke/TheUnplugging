#pragma once

using Layermask = unsigned int;
#include <glm/mat4x4.hpp>

struct RenderParams {
	Layermask layermask;
	struct CommandBuffer& command_buffer;
	glm::mat4 projection;
	glm::mat4 view;

	unsigned int width = 0;
	unsigned int height = 0;
	void set_shader_scene_params(struct Shader&);

	RenderParams(struct CommandBuffer&);
};

struct UpdateParams {
	Layermask layermask;
	struct Input& input;
	double delta_time = 0;

	UpdateParams(struct Input&);
};

struct System {
	virtual void render(struct World& world, struct RenderParams& params) {};
	virtual void update(struct World& world, struct UpdateParams& params) {};
	virtual ~System() {}
};