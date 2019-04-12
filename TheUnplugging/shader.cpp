#include "shader.h"
#include "vfs.h"
#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "layermask.h"

Shader* load_Shader(World& world, const std::string& vfilename, const std::string& ffilename, bool supports_instancing, bool instanced) {


	std::vector<Shader*> existing_shaders = world.filter<Shader>(any_layer);
	for (int i = 0; i < existing_shaders.size(); i++) {
		if (existing_shaders[i]->v_filename == vfilename && existing_shaders[i]->f_filename == ffilename) {
			return existing_shaders[i]; 
		}
	}

	auto id = world.make_ID();
	auto e = world.make<Entity>(id);
	auto shad = world.make<Shader>(id);

	shad->v_filename = vfilename;
	shad->f_filename = ffilename;

	shad->supports_instancing = instanced;
	shad->instanced = instanced;

	shad->load_in_place(world);

	return shad;
}

Shader::~Shader() {
	glDeleteProgram(id);
}

GLint make_shader(const std::string& filename, const std::string& source, GLenum kind, char* info_log) {
	auto c_source = source.c_str();
	auto shader = glCreateShader(kind);

	glShaderSource(shader, 1, &c_source, NULL);
	glCompileShader(shader);

	int sucess = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &sucess);

	if (!sucess) {
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		if (kind == GL_VERTEX_SHADER) {
			throw std::string("(") + filename + std::string(") Vertex shader compilation: ") + std::string(info_log);
		}
		else {
			throw std::string("(") + filename + std::string(") Fragment shader compilation: ") + std::string(info_log);
		}
	}

	return shader;
}

void Shader::on_load(World& world) {
	this->load_in_place(world);
}

void Shader::load_in_place(World& world) {
	std::string vshader_source;
	std::string fshader_source;

	std::string prefix;
	if (this->instanced) prefix = "#version 440 core\n#define IS_INSTANCED\n#line 0\n";
	else prefix = "#version 440 core\n#line 0\n";

	vshader_source = prefix;
	fshader_source = prefix;

	{
		File vshader_f(world.level, v_filename);
		File fshader_f(world.level, f_filename);

		vshader_source += vshader_f.read();
		fshader_source += fshader_f.read();
	}

	int sucess = 0;
	char info_log[512];

	auto vertex_shader = make_shader(v_filename, vshader_source, GL_VERTEX_SHADER, info_log);
	auto fragment_shader = make_shader(f_filename, fshader_source, GL_FRAGMENT_SHADER, info_log);

	auto id = glCreateProgram();

	glAttachShader(id, vertex_shader);
	glAttachShader(id, fragment_shader);

	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &sucess);

	if (!sucess) {
		glGetProgramInfoLog(id, 512, NULL, info_log);
		throw std::string("(") + v_filename + std::string(") Shader linkage : ") + std::string(info_log);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	this->id = id;

	this->irradianceMap = this->location("irradianceMap");
	this->prefilterMap = this->location("prefilterMap");
	this->brdfLUT = this->location("brdfLUT");
	this->model = this->location("model");
	this->projection = this->location("projection");
	this->view = this->location("view");
	this->viewPos = this->location("viewPos");
	this->dirLight_color = this->location("dirLight.direction");
	this->dirLight_direction = this->location("dirLight.color");

	this->v_time_modified = world.level.time_modified(v_filename);
	this->f_time_modified = world.level.time_modified(f_filename);

	if (this->supports_instancing && this->instanced) {
		auto instanced_version = load_Shader(world, v_filename, f_filename, true, true);
		this->instanced_version = std::unique_ptr<Shader>(instanced_version);
	}
}

void Shader::bind() {
	glUseProgram(id);
}

Uniform Shader::location(const std::string& name) {
	return Uniform(name, glGetUniformLocation(id, name.c_str()));
}

Uniform::Uniform(const std::string& name, int id) {
	this->name = name;
	this->id = id;
}

void Uniform::set_mat4(glm::mat4& value) {
	glUniformMatrix4fv(id, 1, false, glm::value_ptr(value));
}

void Uniform::set_vec3(glm::vec3& value) {
	glUniform3fv(id, 1, glm::value_ptr(value));
}

void Uniform::set_vec2(glm::vec2& value) {
	glUniform2fv(id, 1, glm::value_ptr(value));
}

void Uniform::set_int(int value) {
	glUniform1i(id, value);
}

void Uniform::set_float(float value) {
	glUniform1f(id, value);
}