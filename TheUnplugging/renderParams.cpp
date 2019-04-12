#include "system.h"
#include "shader.h"

void RenderParams::set_shader_scene_params(Shader& shader) {
	shader.projection.set_mat4(this->projection);
	shader.view.set_mat4(this->view);
}