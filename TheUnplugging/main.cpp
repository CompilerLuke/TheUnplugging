#include <iostream>
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

using Velocity = glm::vec3;

int main() {
	Window window;
	window.title = "The Unplugging";
	window.init();

	World world;
	world.level.set_level("C:\\Users\\User\\Desktop\\TopCCompiler\\TopCompiler\\Fernix\\assets\\level2\\");
	world.add(new Store<Velocity>(10));
	world.add(new Store<Shader>(10));
	world.add(new Store<Model>(10));
	world.add(new Store<ModelRenderer>(10));
	world.add(new Store<Transform>(10));
	world.add(new Store<LocalTransform>(10));

	world.add(new ModelRendererSystem());
	world.add(new LocalTransformSystem());

	auto shader = load_Shader(world, "shaders/pbr.vert", "shaders/gizmo.frag");
	auto model = load_Model(world, "HOVERTANK.fbx");

	CommandBuffer cmd_buffer;
	RenderParams render_params(cmd_buffer);
	UpdateParams update_params;

	auto id = world.make_ID();
	auto e = world.make<Entity>(id);
	auto trans = world.make<Transform>(id);
	trans->position.z = -5;

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

	auto model_render = world.make<ModelRenderer>(id);
	model_render->model_id = world.id_of(model);
	model_render->set_materials(world, materials);

	glEnable(GL_DEPTH_TEST);

	while (!window.should_close()) {
		temporary_allocator.clear();
		cmd_buffer.clear();

		update_params.layermask = game_layer;
		render_params.layermask = game_layer;
		render_params.view = glm::mat4();
		render_params.projection = glm::perspective(
			glm::radians(60.0f), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
			(float)window.width / (float)window.height,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
			0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
			100.0f             // Far clipping plane. Keep as little as possible.
		);

		world.update(update_params);
		world.render(render_params);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cmd_buffer.submit_to_gpu(world, render_params);
		window.swap_buffers();
	}
	
	return 0;
}