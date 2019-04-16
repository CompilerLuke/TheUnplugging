#include "physics.h"
#include <BtWrapper.h>
#include "ecs.h"
#include "transform.h"
#include <algorithm>

PhysicsSystem::PhysicsSystem() 
: bt_wrapper(make_BulletWrapper()) {
}

PhysicsSystem::~PhysicsSystem() {
	free_BulletWrapper(bt_wrapper);
}

void PhysicsSystem::update(World& world, UpdateParams& params) {
	step_BulletWrapper(bt_wrapper);

	for (ID id : world.filter<RigidBody, Transform>(params.layermask)) {
		auto rb = world.by_id<RigidBody>(id);
		auto trans = world.by_id<Transform>(id);

		if (!rb->bt_rigid_body) {
			RigidBodySettings settings;
			settings.origin.x = trans->position.x;
			settings.origin.y = trans->position.y;
			settings.origin.z = trans->position.z;

			btCollisionShape* shape;
			if (world.by_id<SphereCollider>(id)) {
				auto collider = world.by_id<SphereCollider>(id);
				shape = make_SphereShape(collider->radius * trans->scale.x);
				if (rb->continous) {
					settings.sweep_radius = collider->radius * trans->scale.x;
				}
			}
			else if (world.by_id<BoxCollider>(id)) {
				auto collider = world.by_id<BoxCollider>(id);
				Compat_Vec3 size;
				size.x = collider->scale.x * trans->scale.x;
				size.y = collider->scale.y * trans->scale.y;
				size.z = collider->scale.z * trans->scale.z;
				shape = make_BoxShape(size);
				if (rb->continous) {
					settings.sweep_radius = std::max(size.z, std::max(size.x, size.y));
				}
			}
			else if (world.by_id<CapsuleCollider>(id)) {
				auto collider = world.by_id<CapsuleCollider>(id);
				shape = make_CapsuleShape(collider->radius * trans->scale.x, collider->height * trans->scale.y);
				if (rb->continous) {
					settings.sweep_radius = collider->radius * trans->scale.x + collider->height * trans->scale.y;
				}
			}
			else if (world.by_id<PlaneCollider>(id)) {
				auto collider = world.by_id<PlaneCollider>(id);
				Compat_Vec3 normal;
				normal.x = collider->normal.x;
				normal.y = collider->normal.y;
				normal.z = collider->normal.z;
				shape = make_PlaneShape(normal);
			}
			
			settings.id = id;
			settings.mass = rb->mass;
			settings.lock_rotation = rb->override_rotation;

			rb->bt_rigid_body = make_RigidBody(bt_wrapper, &settings);
		}

		if (rb->mass == 0) continue;

		BulletWrapperTransform trans_of_rb;

		transform_of_RigidBody(rb->bt_rigid_body, &trans_of_rb);

		if (!rb->override_position) 
			trans->position = (glm::vec3&)trans_of_rb.position;
		else 
			trans_of_rb.position = (Compat_Vec3&)trans->position;
		
		if (!rb->override_velocity_x)
			rb->velocity.x = trans_of_rb.velocity.x;
		else
			trans_of_rb.velocity.x = rb->velocity.x;

		if (!rb->override_position)
			rb->velocity.z = trans_of_rb.velocity.z;
		else
			trans_of_rb.velocity.z = rb->velocity.z;

		set_transform_of_RigidBody(rb->bt_rigid_body, &trans_of_rb);
	}
}


