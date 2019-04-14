#include "lights.h"
#include "ecs.h"

DirLight* get_dir_light(World& world, Layermask mask) {
	return world.filter<DirLight>(mask)[0];
}