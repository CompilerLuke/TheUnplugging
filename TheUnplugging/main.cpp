#include <iostream>
#include "frameBuffer.h"
#include "ecs.h"
#include "layermask.h"
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "window.h"
#include "shader.h"
#include "vfs.h"
#include "model.h"
#include "temporary.h"
#include "draw.h"
#include <glm/gtc/matrix_transform.hpp>
#include "transform.h"
#include "camera.h"
#include "input.h"
#include "flyover.h"
#include "game_time.h"
#include "texture.h"
#include "lights.h"
#include "renderPass.h"
#include "ibl.h"
#include "reflection.h"

struct Hey {
	std::vector<glm::vec3> position;

	REFLECT()
};

REFLECT_STRUCT_BEGIN(Hey)
REFLECT_STRUCT_MEMBER(position)
REFLECT_STRUCT_END()

int main() {
	Window window;
	window.title = "The Unplugging";
	window.init();

	Input input(window);
	Time time;

	World world;
	world.level.set_level("C:\\Users\\User\\Desktop\\TopCCompiler\\TopCompiler\\Fernix\\assets\\level2\\");
	world.add(new Store<Entity>(100));
	world.add(new Store<Hey>(10));
	world.add(new Store<Shader>(20));
	world.add(new Store<Model>(10));
	world.add(new Store<ModelRenderer>(10));
	world.add(new Store<Transform>(10));
	world.add(new Store<LocalTransform>(10));
	world.add(new Store<Camera>(3));
	world.add(new Store<Flyover>(1));
	world.add(new Store<Texture>(20));
	world.add(new Store<Cubemap>(10));
	world.add(new Store<DirLight>(2));
	world.add(new Store<Skybox>(1));

	world.add(new CameraSystem());
	world.add(new FlyOverSystem());
	world.add(new ModelRendererSystem());
	world.add(new LocalTransformSystem());
	
	auto shader = load_Shader(world, "shaders/pbr.vert", "shaders/gizmo.frag");
	auto texture = load_Texture(world, "normal.jpg");
	auto model = load_Model(world, "HOVERTANK.fbx");
	auto skybox = load_Skybox(world, "Tropical_Beach_3k.hdr");

	{
		auto id = world.make_ID();
		auto e = world.make<Entity>(id);
		auto trans = world.make<Transform>(id);
		auto dir_light = world.make<DirLight>(id);
	}

	CommandBuffer cmd_buffer;
	MainPass main_pass(world, window);

	RenderParams render_params(cmd_buffer, main_pass);
	UpdateParams update_params(input);

	auto model_renderer_id = world.make_ID();

	{
		auto id = model_renderer_id;
		auto e = world.make<Entity>(id);
		auto trans = world.make<Transform>(id);
		trans->position.z = -5;
	}

	{
		auto id = world.make_ID();
		auto e = world.make<Entity>(id);
		auto trans = world.make<Transform>(id);
		auto camera = world.make <Camera>(id);
		auto flyover = world.make<Flyover>(id);
		auto hey = world.make<Hey>(id);
		hey->position.push_back(glm::vec3(1.0));

		auto typeDesc = reflect::TypeResolver<Hey>::get();
		typeDesc->dump(hey);
	}

	std::vector<Param> params = {
		make_Param_Vec3(shader->location("color"), glm::vec3(0.0f, 0.0f, 0.8f))
	};

	Material material = {
		std::string("DefaultMaterial"),
		world.id_of(shader),
		params,
		&default_draw_state
	};
	std::vector<Material> materials = {material};

	auto model_render = world.make<ModelRenderer>(model_renderer_id);
	model_render->model_id = world.id_of(model);
	model_render->set_materials(world, materials);

	glEnable(GL_DEPTH_TEST);

	while (!window.should_close()) {
		temporary_allocator.clear();
		cmd_buffer.clear();

		input.clear();
		window.poll_inputs();

		update_params.layermask = game_layer;
		time.update_time(update_params);

		render_params.layermask = game_layer;
		render_params.width = window.width;
		render_params.height = window.height;
		render_params.dir_light = get_dir_light(world, render_params.layermask);
		render_params.skybox = skybox;

		world.update(update_params);
		world.render(render_params);

		glViewport(0, 0, render_params.width, render_params.height);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cmd_buffer.submit_to_gpu(world, render_params);
		window.swap_buffers();
	}
	
	return 0;
}