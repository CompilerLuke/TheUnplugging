#include "system.h"
#include "shader.h"
#include "draw.h"
#include "renderPass.h"
#include "rhi.h"

void RenderParams::set_shader_scene_params(Handle<Shader> shader, World& world) {
	this->pass->set_shader_params(shader, world, *this);
}

RenderParams::RenderParams(CommandBuffer* command_buffer, Pass* pass) : 
	command_buffer(command_buffer),
	cam(NULL),
	pass(pass) {

}