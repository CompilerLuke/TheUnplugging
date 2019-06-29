#include "editor.h"
#include "window.h"
#include "rhi.h"

BlockAllocator block_allocator;
MallocAllocator default_allocator;
TemporaryAllocator temporary_allocator(100000);

void register_components_and_systems(World& world) {
	register_default_systems_and_components(world);
}

int main() {
	Window window;
	window.title = "The Unplugging";
	window.init();

	std::string level = "C:\\Users\\User\\Desktop\\TopCCompiler\\TopCompiler\\Fernix\\assets\\level2\\";
	Level::set_level(level);
	
	Editor editor(register_components_and_systems, window);
	editor.run();
	
	return 0;
}