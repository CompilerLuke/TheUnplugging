#include "volumetric.h"
#include "texture.h"
#include "window.h"
#include "primitives.h"
#include "system.h"
#include "shader.h"
#include "lights.h"
#include "transform.h"
#include "camera.h"

FogMap::FogMap(World& world, unsigned int width, unsigned int height) {
	auto tex = world.make_ID();
	
	AttachmentSettings color_attachment(tex);
	color_attachment.min_filter = Nearest;
	color_attachment.mag_filter = Nearest;
	color_attachment.wrap_s = ClampToBorder;
	color_attachment.wrap_t = ClampToBorder;

	FramebufferSettings settings;
	settings.width = width;
	settings.height = height;
	settings.depth_buffer = DepthComponent24;
	settings.color_attachments.append(color_attachment);

	this->fbo = Framebuffer(world, settings);
	this->map = tex;
}

VolumetricPass::VolumetricPass(World& world, Window& window, ID depth_prepass)
	: calc_fog(world, window.width / 2.0f, window.height / 2.0f),
	  depth_prepass(depth_prepass),
	  volume_shader(world.id_of(load_Shader(world, "shaders/screenspace.vert", "shaders/volumetric.frag"))),
	  upsample_shader(world.id_of(load_Shader(world, "shaders/screenspace.vert", "shaders/volumetric_upsample.frag")))
{
}

void VolumetricPass::clear() {
	calc_fog.fbo.bind();
	calc_fog.fbo.clear_color(glm::vec4(1, 1, 1, 1));
	calc_fog.fbo.unbind();
}

void VolumetricPass::render_with_cascade(World& world, RenderParams& render_params, ShadowParams& params) {
	if (!render_params.cam) return;

	auto volume_shader = world.by_id<Shader>(this->volume_shader);
	
	volume_shader->bind();

	calc_fog.fbo.bind();
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_ONE, GL_ONE);

	auto dir_light = get_dir_light(world, game_layer);
	auto dir_light_trans = world.by_id<Transform>(world.id_of(dir_light));
	if (!dir_light_trans) return;

	auto depth_prepass = world.by_id<Texture>(this->depth_prepass);
	auto depth_map = world.by_id<Texture>(params.depth_map);

	depth_prepass->bind_to(0);
	volume_shader->location("depthPrepass").set_int(0);

	depth_map->bind_to(0);
	volume_shader->location("depthMap").set_int(1);

	auto cam_trans = world.by_id<Transform>(world.id_of(render_params.cam));
	if (!cam_trans) return;

	volume_shader->location("camPosition").set_vec3(cam_trans->position);
	volume_shader->location("sunColor").set_vec3(dir_light->color);
	volume_shader->location("sunDirection").set_vec3(dir_light->direction);
	volume_shader->location("sunPosition").set_vec3(dir_light_trans->position);

	volume_shader->location("gCascadeEndClipSpace[0]").set_float(params.in_range.x);
	volume_shader->location("gCascadeEndClipSpace[1]").set_float(params.in_range.y);

	volume_shader->location("toLight").set_mat4(params.to_light);
	volume_shader->location("toWorld").set_mat4(params.to_world);

	volume_shader->location("cascadeLevel").set_int(params.cascade);
	volume_shader->location("endCascade").set_float(render_params.cam->far_plane);

	glm::mat4 ident(1.0);
	volume_shader->location("model").set_mat4(ident);

	render_quad();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	calc_fog.fbo.unbind();
}

void VolumetricPass::render_upsampled(World& world, ID current_frame_id) {
	auto volumetric_map = world.by_id<Texture>(calc_fog.map);
	auto upsample_shader = world.by_id<Shader>(this->upsample_shader);
	auto depth_prepass = world.by_id<Texture>(this->depth_prepass);
	auto current_frame = world.by_id<Texture>(current_frame_id);
	
	upsample_shader->bind();

	glDisable(GL_DEPTH_TEST);

	upsample_shader->location("depthPrepass").set_int(0);
	depth_prepass->bind_to(0);

	upsample_shader->location("volumetricMap").set_int(1);
	volumetric_map->bind_to(1);

	upsample_shader->location("frameMap").set_int(2);
	current_frame->bind_to(2);

	glm::mat4 ident(1.0);
	upsample_shader->location("model").set_mat4(ident);

	render_quad();

	glEnable(GL_DEPTH_TEST);
}
