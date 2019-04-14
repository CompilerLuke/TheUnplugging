#include "system.h"
#include "shader.h"
#include "draw.h"
#include "renderPass.h"

void RenderParams::set_shader_scene_params(Shader& shader, World& world) {
	shader.projection.set_mat4(this->projection);
	shader.view.set_mat4(this->view);

	this->pass.set_shader_params(shader, world, *this);
}

RenderParams::RenderParams(CommandBuffer& command_buffer, Pass& pass) : 
	command_buffer(command_buffer),
	pass(pass) {

}