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
#include "transform.h"
#include "logger.h"

REFLECT_STRUCT_BEGIN(Skybox)
REFLECT_STRUCT_MEMBER(filename)
REFLECT_STRUCT_MEMBER(env_cubemap)
REFLECT_STRUCT_MEMBER(irradiance_cubemap)
REFLECT_STRUCT_MEMBER(prefilter_cubemap)
REFLECT_STRUCT_MEMBER(brdf_LUT)
REFLECT_STRUCT_END()

struct DrawCommandState skybox_draw_state = {
	Cull_None,
	DepthFunc_Lequal,
	false,
	draw_skybox
};

void Skybox::set_ibl_params(Shader& shader, World& world, RenderParams& params) {
	auto bind_to = params.command_buffer->next_texture_index();
	world.by_id<Cubemap>(irradiance_cubemap)->bind_to(bind_to);
	shader.irradianceMap.set_int(bind_to);
	
	bind_to = params.command_buffer->next_texture_index();
	world.by_id<Cubemap>(prefilter_cubemap)->bind_to(bind_to);
	shader.prefilterMap.set_int(bind_to);

	bind_to = params.command_buffer->next_texture_index();
	world.by_id<Texture>(brdf_LUT)->bind_to(bind_to);
	shader.brdfLUT.set_int(bind_to);
}

void Skybox::on_load(World& world) {
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	auto equirectangular_to_cubemap_shader = load_Shader(world, "shaders/eToCubemap.vert", "shaders/eToCubemap.frag");
	auto irradiance_shader = load_Shader(world, "shaders/irradiance.vert", "shaders/irradiance.frag");
	auto cube = load_Model(world, "cube.fbx");


	int width = 2048;
	int height = 2048;

	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	unsigned int hdrTexture;

	// pbr: load the HDR environment map
	// ---------------------------------
	{
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float *data = stbi_loadf(world.level.asset_path(filename).c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glGenTextures(1, &hdrTexture);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			log("Failed to load HDR image.");
		}
	}

	// pbr: setup cubemap to render to and attach to framebuffer
	// ---------------------------------------------------------
	unsigned int envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// pbr: convert HDR equirectangular environment map to cubemap equivalent
	// ----------------------------------------------------------------------
	equirectangular_to_cubemap_shader->bind();
	equirectangular_to_cubemap_shader->location("equirectangularMap").set_int(0);
	equirectangular_to_cubemap_shader->location("projection").set_mat4(captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, width, width); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangular_to_cubemap_shader->location("view").set_mat4(captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cube->meshes[0].buffer.bind();

		log(cube->meshes[0].buffer.length);
		glDrawElements(GL_TRIANGLES, cube->meshes[0].buffer.length, GL_UNSIGNED_INT, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	// --------------------------------------------------------------------------------
	unsigned int irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	irradiance_shader->bind();
	irradiance_shader->location("environmentMap").set_int(0);
	irradiance_shader->location("projection").set_mat4(captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradiance_shader->location("view").set_mat4(captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cube->meshes[0].buffer.bind();
		glDrawElements(GL_TRIANGLES, cube->meshes[0].buffer.length, GL_UNSIGNED_INT, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
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
	prefilter_shader->location("projection").set_mat4(captureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilter_shader->location("roughness").set_float(roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilter_shader->location("view").set_mat4(captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cube->meshes[0].buffer.bind();
			glDrawElements(GL_TRIANGLES, cube->meshes[0].buffer.length, GL_UNSIGNED_INT, 0);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	auto brdf_shader = load_Shader(world, "shaders/brdf_convultion.vert", "shaders/brdf_convultion.frag");

	unsigned int brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	brdf_shader->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render_quad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	auto brdf_LUT_as_texture = make_Texture(world);
	brdf_LUT_as_texture->filename = "brdf_LUT";
	brdf_LUT_as_texture->texture_id = brdfLUTTexture;

	auto env_cubemap_as_texture = make_Cubemap(world);
	env_cubemap_as_texture->filename = "env_cubemap";
	env_cubemap_as_texture->texture_id = envCubemap;

	auto irradiance_cubemap_as_texture = make_Cubemap(world);
	irradiance_cubemap_as_texture->filename = "iraddiance_map";
	irradiance_cubemap_as_texture->texture_id = irradianceMap;

	auto prefilter_cubemap_as_texture = make_Cubemap(world);
	prefilter_cubemap_as_texture->filename = "prefilter_map";
	prefilter_cubemap_as_texture->texture_id = prefilterMap;

	this->brdf_LUT = world.id_of(brdf_LUT_as_texture);
	this->env_cubemap = world.id_of(env_cubemap_as_texture);
	this->irradiance_cubemap = world.id_of(irradiance_cubemap_as_texture);
	this->env_cubemap = world.id_of(env_cubemap_as_texture);
	this->prefilter_cubemap = world.id_of(prefilter_cubemap_as_texture);

	return;
}

Skybox* load_Skybox(World& world, const std::string& filename) {
	for (auto sky : world.filter<Skybox>(any_layer)) {
		if (sky->filename == filename) return sky;
	}

	auto id = world.make_ID();
	auto e = world.make<Entity>(id);
	auto sky = world.make<Skybox>(id);

	sky->filename = filename;
	sky->on_load(world);

	auto skybox_shader = load_Shader(world, "shaders/skybox.vert", "shaders/skybox.frag");
	auto cube_model = load_Model(world, "cube.fbx");

	Material mat;
	mat.shader = world.id_of(skybox_shader);
	mat.params.append(make_Param_Cubemap(skybox_shader->location("environmentMap"), sky->env_cubemap));
	mat.state = &skybox_draw_state;

	auto model_renderer = world.make<ModelRenderer>(id);
	model_renderer->materials.append(mat);
	model_renderer->model_id = world.id_of(cube_model);

	auto trans = world.make<Transform>(id);

	return sky;
}