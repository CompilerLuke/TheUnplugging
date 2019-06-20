#include "ecs.h"
#include "system.h"
#include "transform.h"
#include "camera.h"
#include "input.h"
#include "flyover.h"
#include "shader.h"
#include "model.h"
#include "texture.h"
#include "lights.h"
#include "ibl.h"
#include "physics.h"
#include "lister.h"

void register_default_systems_and_components(World& world) {
	world.add(new Store<Entity>(100));
	world.add(new Store<Transform>(10));
	world.add(new Store<LocalTransform>(10));
	world.add(new Store<Shader>(20));
	world.add(new Store<Model>(10));
	world.add(new Store<ModelRenderer>(10));
	world.add(new Store<Camera>(3));
	world.add(new Store<Flyover>(1));
	world.add(new Store<Texture>(20));
	world.add(new Store<Cubemap>(10));
	world.add(new Store<DirLight>(2));
	world.add(new Store<Skybox>(1));

	world.add(new Store<CapsuleCollider>(10));
	world.add(new Store<SphereCollider>(10));
	world.add(new Store<BoxCollider>(10));
	world.add(new Store<PlaneCollider>(10));
	world.add(new Store<RigidBody>(10));

	world.add(new CameraSystem());
	world.add(new PhysicsSystem());
	world.add(new FlyOverSystem());
	world.add(new ModelRendererSystem());
	world.add(new LocalTransformSystem());
}

