#pragma once

#include "ecs.h"

struct World;

struct System {
	virtual void render(World& world, Layermask layermask) {};
	virtual void update(World& world, Layermask layermask) {};
};