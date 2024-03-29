#include "culling.h"
#include <glm/vec4.hpp>
#include <algorithm>

void aabb_to_verts(AABB* self, glm::vec4* verts) {
	verts[0] = glm::vec4(self->max.x, self->max.y, self->max.z, 1);
	verts[1] = glm::vec4(self->min.x, self->max.y, self->max.z, 1);
	verts[2] = glm::vec4(self->max.x, self->min.y, self->max.z, 1);
	verts[3] = glm::vec4(self->min.x, self->min.z, self->max.z, 1);

	verts[4] = glm::vec4(self->max.x, self->max.y, self->min.z, 1);
	verts[5] = glm::vec4(self->min.x, self->max.y, self->min.z, 1);
	verts[6] = glm::vec4(self->max.x, self->min.y, self->min.z, 1);
	verts[7] = glm::vec4(self->min.x, self->min.z, self->min.z, 1);
}

AABB AABB::apply(const glm::mat4& matrix) {
	AABB new_aabb;
	
	glm::vec4 verts[8];
	aabb_to_verts(this, verts);

	for (int i = 0; i < 8; i++) {
		glm::vec4 v = matrix * verts[i];
		new_aabb.update(v);
	}
	return new_aabb;
}

void AABB::update_aabb(AABB& other) {
	glm::vec4 verts[8];
	aabb_to_verts(&other, verts);

	for (int i = 0; i < 8; i++) {
		this->update(verts[i]);
	}
}

void AABB::update(const glm::vec3& v) {
	this->max.x = std::max(this->max.x, v.x);
	this->max.y = std::max(this->max.y, v.y);
	this->max.z = std::max(this->max.z, v.z);

	this->min.x = std::min(this->min.x, v.x);
	this->min.y = std::min(this->min.y, v.y);
	this->min.z = std::min(this->min.z, v.z);
}




