#include <iostream>
#include "ecs.h"
#include "layermask.h"
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "window.h"
#include "shader.h"
#include "vfs.h"
#include "model.h"

using Velocity = glm::vec3;

int main() {
	set_level("C:\\Users\\User\\Desktop\\TopCCompiler\\TopCompiler\\Fernix\\assets\\level2\\");
	
	Window window;
	window.title = "The Unplugging";
	window.init();

	{
		World world;
		world.add(new Store<Velocity>(10));
		world.add(new Store<Shader>(10));

		auto shader = load_Shader(world, "shaders/pbr.vert", "shaders/pbr.frag");
		auto shader2 = load_Shader(world, "shaders/pbr.vert", "shaders/pbr.frag");
	}

	while (!window.should_close()) {
		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		window.swap_buffers();
	}
	
	return 0;
}