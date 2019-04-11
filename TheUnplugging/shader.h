#pragma once

#include <string>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "ecs.h"

struct Shader;

struct Uniform {
	Uniform(const std::string& name = "", int id = 0);

	std::string name;
	int id;

	void set_mat4(glm::mat4&);
	void set_vec3(glm::vec3&);
	void set_vec2(glm::vec2&);
	void set_int(int);
	void set_float(float);
};

struct Shader {
	std::string v_filename;
	std::string f_filename;
	unsigned int id;

	Uniform irradianceMap;
	Uniform prefilterMap;
	Uniform shadowMaskMap;
	Uniform brdfLUT;
	Uniform model;
	Uniform projection;
	Uniform view;
	Uniform viewPos;
	Uniform dirLight_direction;
	Uniform dirLight_color;

	long v_time_modified = 0;
	long f_time_modified = 0;

	bool supports_instancing = false;
	bool instanced = false;
	std::unique_ptr<Shader> instanced_version = NULL;

	void load_in_place(World&);
	void bind();

	Shader() {};
	~Shader();

	Uniform location(const std::string&);
};


struct ShaderReloadSystem {
	void render();
	void update();
};


Shader* load_Shader(World&, const std::string& vfilename, const std::string& ffilename, bool supports_instancing = false, bool instanced = false);