#pragma once

using Layermask = unsigned int;
#include <glm/mat4x4.hpp>

struct RenderParams {
	Layermask layermask;
	struct CommandBuffer* command_buffer;
	struct Pass* pass;
	struct Skybox* skybox;
	struct DirLight* dir_light;

	glm::mat4 projection;
	glm::mat4 view;
	struct Camera* cam = NULL;

	unsigned int width = 0;
	unsigned int height = 0;
	void set_shader_scene_params(struct Shader&, struct World&);

	RenderParams(struct CommandBuffer*, struct Pass*);
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

void register_default_systems_and_components(World& world);