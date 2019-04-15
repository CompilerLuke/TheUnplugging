#include "ibl.h"
#include "draw.h"
#include "materialSystem.h"
#include "ecs.h"
#include "model.h"
#include "texture.h"
#include "frameBuffer.h"
#include "glad/glad.h"
#include <glm/gtc/matrix_transform.hpp>
#include "model.h"
#include "primitives.h"

struct DrawState skybox_draw_state = {
	Cull_None,
	DepthFunc_Lequal,
	draw_skybox,
	true
};

SkyboxSystem::SkyboxSystem(World& world) {
	skybox_shader = world.id_of(load_Shader(world, "shaders/skybox.vert", "shaders/skybox.frag"));
}

void SkyboxSystem::render(World& world, RenderParams& params) {
	auto cube_model = world.by_id<Model>(this->cube);
	auto skybox_shader = world.by_id<Shader>(this->skybox_shader);

	auto identity_matrix = TEMPORARY_ALLOC(glm::mat4, 1.0);

	for (Skybox* self : world.filter<Skybox>(params.layermask)) {
		Material mat;
		mat.shader = this->skybox_shader;
		mat.params.push_back(make_Param_Cubemap(skybox_shader->location("environmentMap"), self->env_cubemap));
		mat.state = &skybox_draw_state;
	
		std::vector<Material> materials;
		materials.push_back(mat);

		cube_model->render(world.id_of(self), identity_matrix, materials, params);
	}
}

void Skybox::set_ibl_params(Shader& shader, World& world, RenderParams& params) {
	auto bind_to = params.command_buffer.next_texture_index();
	world.by_id<Cubemap>(irradiance_cubemap)->bind_to(bind_to);
	shader.irradianceMap.set_int(bind_to);
	
	bind_to = params.command_buffer.next_texture_index();
	world.by_id<Cubemap>(prefilter_cubemap)->bind_to(bind_to);

	bind_to = params.command_buffer.next_texture_index();
	world.by_id<Cubemap>(brdf_LUT)->bind_to(bind_to);
	shader.brdfLUT.set_int(bind_to);
}

void Skybox::on_load(World& world) {
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	auto equirectangular_to_cubemap_shader = load_Shader(world, "shaders/eToCubemap.vert", "shaders/eToCubemap.frag");
	auto cube = load_Model(world, "cube.fbx");

	unsigned int capture_FBO;
	unsigned int capture_RBO;

	constexpr int width = 2048;
	constexpr int height = 2048;

	glGenFramebuffers(1, &capture_FBO);
	glGenFramebuffers(1, &capture_RBO);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	
	auto hdr_texture = load_Texture(world, filename);

	unsigned int env_cubemap;
	glGenTextures(1, &env_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);

	for (int i = 0; i < 6; i++) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			width,
			height,
			0,
			GL_RGB,
			GL_FLOAT,
			NULL
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glm::mat4 capture_projection = glm::perspective(
		glm::radians(90.0f),
		1.0f,
		0.1f,
		10.0f
	);

	glm::mat4 capture_views[6] = {
		glm::lookAt(glm::vec3(0), glm::vec3(1,0,0), glm::vec3(0,-1,0)),
		glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0), glm::vec3(0,-1,0)),
		glm::lookAt(glm::vec3(0), glm::vec3(0,1,0), glm::vec3(0,0,1)),
		glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0), glm::vec3(0,0,-1)),
		glm::lookAt(glm::vec3(0), glm::vec3(0,0,1), glm::vec3(0,-1,0)),
		glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,-1,0))
	};

	equirectangular_to_cubemap_shader->bind();
	equirectangular_to_cubemap_shader->location("equirectangularMap").set_int(0);
	equirectangular_to_cubemap_shader->location("projection"), capture_projection;

	hdr_texture->bind_to(0);

	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);

	for (int i = 0; i < 6; i++) {
		equirectangular_to_cubemap_shader->location("view").set_mat4(capture_views[i]);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
			env_cubemap,
			0
		);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cube->meshes[0].buffer.bind();
		glDrawElements(GL_TRIANGLES, cube->meshes[0].buffer.length, GL_UNSIGNED_INT, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	auto irradiance_shader = load_Shader(world, "shaders/irradiance.vert", "shaders/irradiance.frag");

	unsigned int irradiance_map;

	glGenTextures(1, &irradiance_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);

	for (int i = 0; i < 6; i++) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
			0,
			GL_RGB16F,
			32, 32,
			0,
			GL_RGB,
			GL_FLOAT,
			NULL
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	irradiance_shader->bind();
	irradiance_shader->location("environmentMap").set_int(0);
	irradiance_shader->location("projection").set_mat4(capture_projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);

	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);

	for (int i = 0; i < 6; i++) {
		irradiance_shader->location("view").set_mat4(capture_views[i]);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
			irradiance_map,
			0
		);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int prefilter_map;
	glGenTextures(1, &prefilter_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);

	for (int i = 0; i < 6; i++) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			256, 256, 0,
			GL_RGB, GL_FLOAT, NULL
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	auto prefilter_shader = load_Shader(world, "shaders/prefilter.vert", "shaders/prefilter.frag");

	prefilter_shader->bind();
	prefilter_shader->location("environmentMap").set_int(0);
	prefilter_shader->location("projection").set_mat4(capture_projection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);

	int maxMipLevels = 6;

	Uniform roughness_id = prefilter_shader->location("roughness");
	Uniform view_id = prefilter_shader->location("view");

	for (int mip = 0; mip < maxMipLevels; mip++) {
		int mip_width = (int)(256 * pow(0.5, mip));
		int mip_height = (int)(256 * pow(0.5, mip));

		glBindRenderbuffer(GL_RENDERBUFFER, capture_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
		glViewport(0, 0, mip_width, mip_height);

		float roughness = (float)mip / ((float)maxMipLevels - 1);
		roughness_id.set_float(roughness);

		for (int i = 0; i < 6; i++) {
			view_id.set_mat4(capture_views[i]);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
				prefilter_map,
				mip
			);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cube->meshes[0].buffer.bind();
			glDrawElements(GL_TRIANGLES, cube->meshes[0].buffer.length, GL_UNSIGNED_INT, NULL);
		}
	}

	auto brdf_shader = load_Shader(world, "shaders/brdf_convultion.vert", "shaders/brdf_convultion.frag");

	unsigned int brdf_LUT_TEXTURE;
	glGenTextures(1, &brdf_LUT_TEXTURE);
	
	glBindTexture(GL_TEXTURE_2D, brdf_LUT_TEXTURE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 512, 512, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, capture_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_LUT_TEXTURE, 0);

	glViewport(0, 0, 512, 512);
	brdf_shader->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_quad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	auto brdf_LUT_as_texture = make_Texture(world);
	brdf_LUT_as_texture->filename = "brdf_LUT";
	brdf_LUT_as_texture->texture_id = brdf_LUT_TEXTURE;

	auto env_cubemap_as_texture = make_Cubemap(world);
	env_cubemap_as_texture->filename = "env_cubemap";
	env_cubemap_as_texture->texture_id = env_cubemap;

	auto irradiance_cubemap_as_texture = make_Cubemap(world);
	irradiance_cubemap_as_texture->filename = "iraddiance_map";
	irradiance_cubemap_as_texture->texture_id = irradiance_map;

	auto prefilter_cubemap_as_texture = make_Cubemap(world);
	prefilter_cubemap_as_texture->filename = "prefilter_map";
	prefilter_cubemap_as_texture->texture_id = prefilter_cubemap;

	this->brdf_LUT = world.id_of(brdf_LUT_as_texture);
	this->env_cubemap = world.id_of(env_cubemap_as_texture);
	this->irradiance_cubemap = world.id_of(irradiance_cubemap_as_texture);
	this->env_cubemap = world.id_of(env_cubemap_as_texture);
	this->prefilter_cubemap = world.id_of(prefilter_cubemap_as_texture);
}

