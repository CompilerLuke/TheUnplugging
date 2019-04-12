#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include "ecs.h"

struct Transform {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale = glm::vec3(1.0f);
	
	glm::mat4 compute_model_matrix();
};

struct LocalTransform {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale = glm::vec3(1.0f);
	ID owner;

	void calc_global_transform(struct World&);
};

struct LocalTransformSystem : System {
	void update(World&, UpdateParams&) override;
};