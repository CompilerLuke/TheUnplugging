#include "ecs.h"

ID World::make_ID() {
	if (freed_entities.size() > 0) {
		ID id = freed_entities[freed_entities.size() - 1];
		freed_entities.pop_back();
		return id;
	}
	return current_id++;
}

void World::free_ID(ID id) {
	freed_entities.push_back(id);
}