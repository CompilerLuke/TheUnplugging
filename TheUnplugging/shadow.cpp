#include "shadow.h"
#include "window.h"
#include "shader.h"
#include "system.h"
#include "texture.h"
#include "renderPass.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "draw.h"
#include "lights.h"
#include "transform.h"
#include "primitives.h"

DepthMap::DepthMap(unsigned int width, unsigned int height, World& world) {
	Texture* depthMap = make_Texture(world);
	AttachmentSettings attachment(world.id_of(depthMap));

	FramebufferSettings settings;
	settings.width = width;
	settings.height = height;
	settings.depth_attachment = &attachment;
	settings.depth_buffer = DepthComponent24;

	this->depth_map = world.id_of(depthMap);
	this->depth_map_FBO = Framebuffer(world, settings);
}

ShadowPass::ShadowPass(Window& window, World& world, ID depth_prepass) :
	deffered_map_cascade(4096, 4096, world),
	ping_pong_shadow_mask(window, world),
	shadow_mask(window, world)
{
	this->depth_prepass = depth_prepass;
}

ShadowMask::ShadowMask(Window& window, World& world) {
	Texture* tex = make_Texture(world);
	AttachmentSettings color_attachment(world.id_of(tex));
	color_attachment.mag_filter = Linear;
	color_attachment.min_filter = Linear;
	color_attachment.wrap_s = Repeat;
	color_attachment.wrap_t = Repeat;

	FramebufferSettings settings;
	settings.width = window.width;
	settings.height = window.height;
	settings.color_attachments.push_back(color_attachment);
	settings.depth_buffer = DepthComponent24;

	this->shadow_mask_map_fbo = Framebuffer(world, settings);
}

void ShadowMask::set_shadow_params(Shader& shader, World& world, RenderParams& params) {
	auto tex = world.by_id<Texture>(shadow_mask_map);
	auto bind_to = params.command_buffer.next_texture_index();
	tex->bind_to(bind_to);
	shader.shadowMaskMap.set_int(bind_to);
}

struct OrthoProjInfo {
	float endClipSpace;
	glm::mat4 toLight;
	glm::mat4 toWorld;
	glm::mat4 lightProjection;
};

constexpr int num_cascades = 4;

void calc_ortho_proj(RenderParams& params, glm::mat4& light_m, float width, float height, OrthoProjInfo shadowOrthoProjInfo[num_cascades]) {
	auto& cam_m = params.view;
	auto cam_inv_m = glm::inverse(cam_m);
	auto& proj_m = params.projection;

	if (!params.cam) throw "Can not calculate cascades without camera";

	float cascadeEnd[] = {
		params.cam->near_plane,
		20.0f,
		90.0f,
		150.0f,
		params.cam->far_plane,
	};

	for (int i = 0; i < num_cascades; i++) {
		auto proj = glm::perspective(
			glm::radians(params.cam->fov),
			(float)params.width / (float)params.height,
			cascadeEnd[i],
			cascadeEnd[i + 1]
		);

		glm::vec4 frustumCorners[8] = {
			glm::vec4(1,1,1,1),
			glm::vec4(-1,1, 1,1),
			glm::vec4(1,-1,1,1),
			glm::vec4(-1,-1,1,1),

			glm::vec4(1,1,-1,1),
			glm::vec4(-1,1,-1,1),
			glm::vec4(1,-1,-1,1),
			glm::vec4(-1,-1,-1,1)
		};

		auto frust_to_world = glm::inverse(proj * cam_m);

		glm::vec4 farFrustumCorners[5] = {
			glm::vec4(1,1,1,1),
			glm::vec4(-1,1,1,1),
			glm::vec4(1,-1,1,1),
			glm::vec4(-1,-1,1,1),
			glm::vec4(0,0,1,1) //centroid
		};
	
		for (int j = 0; j < 5; j++) {
			auto vW = frust_to_world * farFrustumCorners[j];
			vW /= vW.w;

			farFrustumCorners[j] = light_m * vW;
		}

		float radius = 0;

		for (int j = 0; j < 4; j++) {
			glm::vec3 a(farFrustumCorners[j]);
			glm::vec3 b(farFrustumCorners[4]);
			radius = std::max(radius, glm::length(a - b));
		}

		glm::vec4 centroid = farFrustumCorners[4];

		float minX = centroid.x - radius;
		float maxX = centroid.x + radius;
		float minY = centroid.y - radius;
		float maxY = centroid.z + radius;
		float minZ = 1;
		float maxZ = 200; //todo make less hardcoded

		glm::mat4 light_projection_matrix = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

		shadowOrthoProjInfo[i].toWorld = glm::inverse(proj_m * cam_m);
		shadowOrthoProjInfo[i].toLight = light_projection_matrix * light_m;
		shadowOrthoProjInfo[i].lightProjection = light_projection_matrix;
	}
}

void DepthMap::render_maps(World& world, RenderParams& params, glm::mat4 projection_m, glm::mat4 view_m) {
	CommandBuffer command_buffer;

	for (auto cmd : params.command_buffer.commands) { //makes copy of command
		auto shad = world.by_id<Shader>(cmd.material->shader);
		if (!shad) continue;

		if (shad->v_filename == "shaders/skybox.vert") continue; //currently different vertex shader is not supported

		auto depth_material = TEMPORARY_ALLOC(Material);
		depth_material->shader = depth_shader;
		depth_material->state = &default_draw_state;
		cmd.material = depth_material;

		command_buffer.submit(cmd);
	}

	RenderParams new_params = params;
	new_params.command_buffer = command_buffer;
	new_params.view = view_m;
	new_params.projection = projection_m;

	this->depth_map_FBO.bind();
	glEnable(GL_DEPTH_TEST);

	this->depth_map_FBO.clear_depth(glm::vec4(0, 0, 0, 1));
	
	command_buffer.submit_to_gpu(world, new_params);

	this->depth_map_FBO.unbind();
}

void ShadowPass::render(World& world, RenderParams& params) {
	auto dir_light = get_dir_light(world, params.layermask);
	if (!dir_light) return;
	auto dir_light_id = world.id_of(dir_light);

	auto dir_light_trans = world.by_id<Transform>(dir_light_id);
	if (!dir_light_trans) return;

	glm::mat4 view_matrix(1.0);
	view_matrix = glm::translate(view_matrix, -dir_light_trans->position);
	view_matrix = view_matrix * glm::mat4_cast(dir_light_trans->rotation);

	auto width = this->deffered_map_cascade.width;
	auto height = this->deffered_map_cascade.height;

	OrthoProjInfo info[num_cascades];
	calc_ortho_proj(params, view_matrix, width, height, info);

	shadow_mask.shadow_mask_map_fbo.bind();
	shadow_mask.shadow_mask_map_fbo.clear_color(glm::vec4(0, 0, 0, 1));

	float last_clip_space = -1.0f;

	auto shadow_mask_shader = world.by_id<Shader>(this->shadow_mask_shader);
	auto depth_prepass = world.by_id<Texture>(this->depth_prepass);
	auto shadow_map = world.by_id<Texture>(this->shadow_mask.shadow_mask_map);

	for (int i = 0; i < num_cascades; i++) {
		auto& proj_info = info[i];
		deffered_map_cascade.render_maps(world, params, proj_info.lightProjection, view_matrix);

		shadow_mask.shadow_mask_map_fbo.bind();

		glDisable(GL_DEPTH_TEST);

		shadow_mask_shader->bind();

		depth_prepass->bind_to(0);
		shadow_mask_shader->location("depthPrepass").set_int(0);

		shadow_map->bind_to(1);
		shadow_mask_shader->location("depthMap").set_int(1);

		shadow_mask_shader->location("gCascadeEndClipSpace[0]").set_float(last_clip_space);
		shadow_mask_shader->location("gCascadeEndClipSpace[1]").set_float(proj_info.endClipSpace);
	
		glm::mat4 ident_matrix(1.0);
		shadow_mask_shader->location("model").set_mat4(ident_matrix);

		shadow_mask_shader->location("cascadeLevel").set_int(i);
		
		glm::vec2 in_range(last_clip_space, proj_info.endClipSpace);

		last_clip_space = proj_info.endClipSpace;

		render_quad();

		glEnable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	bool horizontal = true;
	bool first_iteration = true;

	glDisable(GL_DEPTH_TEST);

	auto blur_shader = world.by_id<Shader>(this->screenspace_blur_shader);

	constexpr int amount = 10;
	for (int i = 0; i < amount; i++) {
		if (!horizontal) shadow_mask.shadow_mask_map_fbo.bind();
		else ping_pong_shadow_mask.shadow_mask_map_fbo.bind();

		blur_shader->bind();
	}
}