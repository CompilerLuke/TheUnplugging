#include "ecs.h"

//REFLECT_ALIAS(ID, unsigned int) requires strong typedef
//REFLECT_ALIAS(Layermask, unsigned int) requires strong typedef

REFLECT_STRUCT_BEGIN(Entity)
REFLECT_STRUCT_MEMBER(enabled)
REFLECT_STRUCT_MEMBER(layermask)
REFLECT_STRUCT_END()

int global_type_id = 0;

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