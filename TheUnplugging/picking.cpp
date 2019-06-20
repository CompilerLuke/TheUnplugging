#include "picking.h"
#include <glad/glad.h>
#include "editor.h"
#include "input.h"
#include "draw.h"
#include "layermask.h"

int PickingPass::pick(World& world, UpdateParams& params) {
	auto mouse_position = params.input.mouse_position;

	int id = -1;

	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	glReadPixels((int)mouse_position.x, (int)this->height - mouse_position.y, 1, 1, GL_RED_INTEGER, GL_INT, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return id;
}

void PickingPass::render(World& world, RenderParams& params) {
	auto render_mask = picking_layer;

	CommandBuffer new_command_buffer;
	RenderParams new_params(&new_command_buffer, this);
	new_params.layermask = render_mask;

	world.render(new_params);
}

