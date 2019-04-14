#include "renderPass.h"
#include "ecs.h"
#include "window.h"
#include "draw.h"
#include "frameBuffer.h"

MainPass::MainPass(World& world, Window& window)
	: depth_prepass(window.width, window.height, world),
	shadow_pass(window, world, depth_prepass.depth_map)
{
	frame_map = world.make_ID();

	AttachmentSettings attachment(frame_map);
	FramebufferSettings settings;
	settings.width = window.width;
	settings.height = window.height;
	settings.depth_buffer = DepthComponent24;
	settings.color_attachments.push_back(attachment);

	device.width = window.width;
	device.height = window.height;
	device.multisampling = 4;
	device.clear_colour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	current_frame = Framebuffer(world, settings);
}

void MainPass::set_shader_params(struct Shader& shader, World& world, RenderParams& params) {
	shadow_pass.set_shadow_params(shader, world, params);
}

void MainPass::render(World& world, RenderParams& params) {
	device.width = params.width;
	device.height = params.height;

	params.command_buffer.clear();

	world.render(params);

	current_frame.bind();
	current_frame.clear_color(glm::vec4(0, 0, 0, 1));
	current_frame.clear_depth(glm::vec4(0, 0, 0, 1));

	params.command_buffer.submit_to_gpu(world, params);

	current_frame.unbind();

	device.bind();

}