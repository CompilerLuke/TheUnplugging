#pragma once

#include "shader.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "id.h"

enum Param_Type {
	Param_Vec3,
	Param_Vec2,
	Param_Mat4x4,
	Param_Image,
	Param_Cubemap,
	Param_Int
};

struct Param {
	Uniform loc;
	Param_Type type;
	union {
		glm::vec3 vec3;
		glm::vec2 vec2;
		glm::mat4 matrix;
		ID image;
		ID cubemap;
		int integer;
	};

	Param();
};

Param make_Param_Vec3(const Uniform& loc, glm::vec3);
Param make_Param_Cubemap(const Uniform& loc, ID);

struct Material {
	std::string name;
	ID shader;
	std::vector<Param> params;
	struct DrawState* state;
};


Material* material_by_name(std::vector<Material>&, const std::string&);