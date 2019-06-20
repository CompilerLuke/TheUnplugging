#include "editor.h"
#include "window.h"

BlockAllocator block_allocator;
MallocAllocator default_allocator;
TemporaryAllocator temporary_allocator(10000);

void register_components_and_systems(World& world) {
	register_default_systems_and_components(world);
}

int main() {
	Window window;
	window.title = "The Unplugging";
	window.init();

	std::string level = "C:\\Users\\User\\Desktop\\TopCCompiler\\TopCompiler\\Fernix\\assets\\level2\\";
	
	Editor editor(level, register_components_and_systems, window);
	editor.run();
	
	return 0;
}