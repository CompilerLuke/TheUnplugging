#pragma once

#include <glm/vec3.hpp>
#include "id.h"

struct DirLight {
	glm::vec3 direction;
	glm::vec3 color;
};

DirLight* get_dir_light(struct World&, Layermask layermask);