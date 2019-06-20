#pragma once

#include <string>
#include "eventDispatcher.h"
#include "id.h"
#include "reflection.h"
#include "vector.h"
#include "ecs.h"

struct EntityEditor { //Editor meta data for this entity
	std::string name;
	vector<ID> children;

	REFLECT()
};

struct Lister {
	std::string filter;

	void render(struct World& world, struct Editor& editor, struct RenderParams& params);
};

std::string name_with_id(struct World&, ID id);