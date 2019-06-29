#include "picking.h"
#include <glad/glad.h>
#include "editor.h"
#include "input.h"
#include "draw.h"
#include "layermask.h"
#include "temporary.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include "temporary.h"
#include "editor.h"
#include "window.h"

PickingPass::PickingPass(Window& params)
: picking_shader(load_Shader("shaders/pbr.vert", "shaders/picking.frag")) {
	
	AttachmentSettings color_attachment(picking_map);
	color_attachment.internal_format = R32I;
	color_attachment.external_format = Red_Int;
	color_attachment.texel_type = Int_Texel;
	color_attachment.min_filter = Nearest;
	color_attachment.mag_filter = Nearest;

	FramebufferSettings settings;
	settings.color_attachments.append(color_attachment);
	settings.width = params.width;
	settings.height = params.height;

	framebuffer = Framebuffer(settings);
}

void PickingPass::set_shader_params(Handle<Shader> shader, World& world, RenderParams& params) {

}

int PickingPass::pick(World& world, UpdateParams& params) {
	auto mouse_position = params.input.mouse_position;

	int id = -1;

	glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer.fbo);
	glReadPixels((int)mouse_position.x, (int)this->framebuffer.width - mouse_position.y, 1, 1, GL_RED_INTEGER, GL_INT, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return id;
}

void PickingPass::render(World& world, RenderParams& params) {
	if (!params.layermask & editor_layer) return;
	
	CommandBuffer cmd_buffer;
	RenderParams new_params(&cmd_buffer, this);
	new_params.width = framebuffer.width;
	new_params.height = framebuffer.height;
	new_params.layermask = picking_layer;

	world.render(new_params);

	Handle<Uniform> loc = location(picking_shader, "id");

	for (DrawCommand& cmd : new_params.command_buffer->commands) {
		vector<Param> params;
		params.allocator = &temporary_allocator;
		params.append(make_Param_Int(loc, cmd.id));

		cmd.material = TEMPORARY_ALLOC(Material, {
			"Picking Material", //todo this will allocate
			picking_shader,
			params,
			cmd.material->state
		});
	}

	framebuffer.clear_color(glm::vec4(0.0, 0.0, 0.0, 1.0));
	framebuffer.clear_depth(glm::vec4(0.0, 0.0, 0.0, 1.0));
	framebuffer.bind();

	cmd_buffer.submit_to_gpu(world, new_params);

	framebuffer.unbind();
}

PickingSystem::PickingSystem(Editor& editor) : editor(editor) {
	this->outline_shader = load_Shader("shaders/outline.vert", "shaders/outline.frag"); //todo move into constructor

	outline_state.order = (DrawOrder)3;
	outline_state.mode = DrawWireframe;
	outline_state.stencil_func = StencilFunc_NotEqual;
	outline_state.stencil_op = Stencil_Keep_Replace;
	outline_state.stencil_mask = 0x00;
	outline_state.depth_func = DepthFunc_None;

	object_state.order = (DrawOrder)4;
	object_state.stencil_func = StencilFunc_Always;
	object_state.stencil_mask = 0xFF;
	object_state.stencil_op = Stencil_Keep_Replace;
	object_state.clear_stencil_buffer = true;

	this->outline_material = {
		"OutlineMaterial",
		outline_shader,
		{},
		&outline_state
	};
}

void PickingSystem::render(World& world, RenderParams& params) {
	if (!params.layermask & editor_layer) return;
	if (params.layermask & picking_layer) return;

	//Render Outline
	int selected = editor.selected_id;
	if (selected == -1) return;

	for (DrawCommand& cmd : params.command_buffer->commands) {
		if (cmd.id != selected) continue;

		{
			Material* mat = TEMPORARY_ALLOC(Material, *cmd.material);
			mat->state = TEMPORARY_ALLOC(DrawCommandState, object_state);
			cmd.material = mat;
		}

		{
			DrawCommand new_cmd = cmd;
			new_cmd.material = TEMPORARY_ALLOC(Material, outline_material);
			params.command_buffer->submit(new_cmd);
		}
	}

	glLineWidth(10.0);
}

