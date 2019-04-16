#pragma once

#include "id.h"
#include "system.h"
#include <glm/vec3.hpp>
#include "BtWrapper.h"

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
	float mass = 0;
	glm::vec3 velocity;
	bool override_position = false;
	bool override_rotation = false;

	bool override_velocity_x = false;
	bool override_velocity_y = false;
	bool override_velocity_z = false;

	bool continous = false;

	struct btRigidBody* bt_rigid_body = NULL;
};

struct PhysicsSystem : System {
	BulletWrapper* bt_wrapper;

	PhysicsSystem();
	~PhysicsSystem();

	void update(struct World&, UpdateParams&) override;
};