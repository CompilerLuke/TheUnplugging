#pragma once

#include "ecs.h"
#include <glm/mat4x4.hpp>
#include "culling.h"
#include "buffer.h"
#include <vector>
#include "temporary.h"
#include <unordered_map>
#include "ecs.h"

enum Cull {Cull_Front, Cull_Back, Cull_None};
enum DepthFunc {DepthFunc_Less, DepthFunc_Lequal};

struct DrawState {
	Cull cull;
	DepthFunc depth_func;
	bool clear_depth_buffer;
	unsigned int order;
};

extern struct DrawState default_draw_state;

#include "materialSystem.h"

using DrawSortKey = long long;

struct DrawCommand {
	DrawSortKey key = 0;
	ID id;
	glm::mat4* model_m;
	AABB* aabb;
	VertexBuffer* buffer;
	Material* material;

	DrawCommand(ID, glm::mat4*, AABB*, VertexBuffer*, Material*);
};

struct CommandBuffer {
	std::vector<DrawCommand, STDTemporaryAllocator<DrawCommand> > commands;
	std::unordered_map<DrawSortKey, int> instanced_buffers;

	unsigned int current_texture_index = 0;

	CommandBuffer();
	~CommandBuffer();

	void submit(DrawCommand&);
	void clear();

	void submit_to_gpu(World&, RenderParams&);
};