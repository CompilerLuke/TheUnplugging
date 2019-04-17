#pragma once

#include <vector>
#include "layermask.h"

#include <vector>
#include <utility>
#include <cassert>

#include <memory>
#include "id.h"
#include "system.h"
#include "vfs.h"
#include "reflection.h"

template<typename T>
struct Slot {
	union {
		Slot<T>* next;
		std::pair<T, ID> object;
	};
	bool is_enabled = false;

	Slot() {
		next = NULL;
	}

	~Slot() {
		if (is_enabled) {
			object.first.~T();
		}
	}
};

constexpr unsigned int max_entities = 1000;

struct Component {

};

struct ComponentStore {
	virtual void free_by_id(ID) {};
	virtual ~ComponentStore() {};
};

extern int global_type_id;
using typeid_t = int;

template <typename T>
constexpr typeid_t type_id() noexcept
{
	static int const type_id = global_type_id++;
	return type_id;
}

template<typename T>
struct Store : ComponentStore {
	T* id_to_obj[max_entities];
	Slot<T>* components;
	Slot<T>* free_slot;
	unsigned int N;

	Store(unsigned int max_number) {
		N = max_number;
		assert(N > 0);

		for (int i = 0; i < max_entities; i++) {
			id_to_obj[i] = NULL;
		}

		components = new Slot<T>[N];
		for (unsigned int i = 0; i < N - 1; i++) {
			components[i].next = &components[i + 1];
		}

		free_slot = &components[0];
	}

	virtual ~Store() {
		delete[] components;
	}

	T* by_id(ID id) {
		if (id >= max_entities) return NULL;
		return id_to_obj[id];
	}

	Component get_by_id(ID id) {
		return this->by_id(id);
	}

	void free_by_id(ID id) {
		auto obj_ptr = this->by_id(id);
		if (!obj_ptr) return;

		obj_ptr->~T();

		id_to_obj[id] = NULL;
		auto slot = (Slot<T>*) obj_ptr;
		slot->next = free_slot;
		slot->is_enabled = false;
		free_slot = slot;
	}

	T* make(ID id) {
		if (free_slot == NULL) { throw "Out of memory for system"; }

		auto current_free_slot = free_slot;
		this->free_slot = current_free_slot->next;

		new (&current_free_slot->object.first) T();
		current_free_slot->object.second = id;
		current_free_slot->is_enabled = true;

		auto obj_ptr = &current_free_slot->object.first;
		this->register_component(id, obj_ptr);
		return obj_ptr;
	}

	void register_component(ID id, T* obj) {
		assert(id < max_entities);
		id_to_obj[id] = obj;
	}
};

struct Entity {
	bool enabled = true;
	Layermask layermask = game_layer;

	REFLECT()
};

struct World {
	static constexpr int components_hash_size = 100;

	std::unique_ptr<ComponentStore> components[components_hash_size];
	std::vector<std::unique_ptr<System>> systems;
	Level level;

	template<typename T>
	constexpr void add(Store<T>* store) {
		assert(get<T>() == NULL);
		components[(size_t)type_id<T>()] = std::unique_ptr<ComponentStore>(store);
	}

	void add(System* system) {
		systems.push_back(std::unique_ptr<System>(system));
	}

	template<typename T>
	constexpr Store<T>* get() {
		return (Store<T>*)(components[(size_t)type_id<T>()].get());
	}

	template<typename T>
	T* make(ID id) {
		return this->get<T>()->make(id);
	}

	ID make_ID();
	void free_ID(ID);

	template<typename T>
	void free_by_id(ID id) {
		this->get<T>()->free_by_id(id);
	}

	template<>
	void free_by_id<Entity>(ID id) {
		for (int i = 0; i < components_hash_size; i++) {
			if (components[i] == NULL) continue;
			components[i]->free_by_id(id);
		}
		this->free_ID(id);
	}

	template<typename T>
	ID id_of(T* ptr) {
		assert(get<T>() != NULL);
		return ((Slot<T>*)ptr)->object.second;
	}

	template<typename T>
	T* by_id(ID id) {
		return get<T>()->by_id(id);
	}

	template<typename T>
	std::vector<T*> filter(Layermask layermask) {
		Store<T>* store = get<T>();
		Store<Entity>* entity_store = get<Entity>();

		std::vector<T*> arr;
		for (unsigned int i = 0; i < store->N; i++) {
			auto slot = &store->components[i];

			if (!slot->is_enabled) continue;

			auto entity = entity_store->by_id(slot->object.second);

			if (entity && entity->enabled && (entity->layermask & layermask)) {
				arr.push_back(&slot->object.first);
			}
		}

		return arr;
	}

	template <typename... T>
	typename std::enable_if<(sizeof...(T) == 0), bool>::type
		has_component(ID id) {
		return true;
	}

	template<typename T, typename... Args>
	bool has_component(ID id) {
		return by_id<T>(id) != NULL && has_component<Args...>(id);
	}

	template<typename A, typename... Args>
	typename std::enable_if<(sizeof...(Args) > 0), std::vector <ID> >::type
		filter(Layermask layermask) {
		std::vector<ID> ids;
		Store<Entity>* entity_store = get<Entity>();

		for (int i = 0; i < max_entities; i++) {
			if (has_component<A, Args...>(i)) {
				auto entity = entity_store->by_id(i);
				if (entity && entity->enabled && entity->layermask & layermask) {
					ids.push_back(i);
				}
			}
		}

		return ids;
	}

	void render(RenderParams& params) {
		for (int i = 0; i < systems.size(); i++) {
			auto system = systems[i].get();
			system->render(*this, params);
		}
	}

	void update(UpdateParams& params) {
		for (int i = 0; i < systems.size(); i++) {
			auto system = systems[i].get();
			system->update(*this, params);
		}
	}

	World() {
		for (int i = 0; i < components_hash_size; i++) {
			components[i] = NULL;
		}
	}

private:
	unsigned int current_id = 0;
	std::vector<ID> freed_entities;
};