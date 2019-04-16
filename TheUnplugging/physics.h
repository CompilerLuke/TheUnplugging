#pragma once

#include "id.h"
#include "system.h"
#include <glm/vec3.hpp>

struct CapsuleCollider {
	float radius;
	float height;
};

struct SphereCollider {
	float radius;
};

struct BoxCollider {
	glm::vec3 scale;
};

struct PlaneCollider {
	glm::vec3 normal;
};

struct RigidBody {
	float mass;
	glm::vec3 velocity;
	bool override_position;
	bool override_rotation;

	bool override_velocity_x;
	bool override_velocity_y;
	bool override_velocity_z;

	struct btRigidBody* bt_rigid_body = NULL;
};

struct PhysicsSystem : System {
	PhysicsSystem();
	~PhysicsSystem();

	void update(struct World&, UpdateParams&) override;
};