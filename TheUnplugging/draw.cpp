#include "draw.h"
#include <glad/glad.h>
#include <algorithm>

REFLECT_ENUM(Cull)
REFLECT_ENUM(DepthFunc)

REFLECT_STRUCT_BEGIN(DrawCommandState)
REFLECT_STRUCT_MEMBER(cull)
REFLECT_STRUCT_MEMBER(depth_func)
REFLECT_STRUCT_MEMBER(clear_depth_buffer)
REFLECT_STRUCT_MEMBER(order)
REFLECT_STRUCT_END()

#include "texture.h"

DrawCommandState default_draw_state = {
	Cull_None,
	DepthFunc_Lequal,
	true,
	draw_opaque
};

DrawCommandState draw_draw_over = {
	Cull_None,
	DepthFunc_Lequal,
	true,
	draw_over,
};

DrawCommand::DrawCommand(ID id, glm::mat4* model, AABB* aabb, VertexBuffer* vertex_buffer, Material* material) :
	id(id), model_m(model), aabb(aabb), buffer(vertex_buffer), material(material)
{}

CommandBuffer::CommandBuffer() {}
CommandBuffer::~CommandBuffer() {}

unsigned int CommandBuffer::next_texture_index() {
	return current_texture_index++;
}

void CommandBuffer::submit(DrawCommand& cmd) {
	commands.push_back(cmd);
}

void CommandBuffer::clear() {
	commands.clear();
	commands.shrink_to_fit();
	//otherwise we would have memory from last frame, which may get overwritten
	commands.reserve(50);
}

bool compare_key(DrawCommand& cmd1, DrawCommand& cmd2) {
	return cmd1.key < cmd2.key;
}

int can_instance(std::vector<DrawCommand, STDTemporaryAllocator<DrawCommand>>& commands, int i, World& world) {
	auto count = 1;

	auto shader = world.by_id<Shader>(commands[i].material->shader);
	if (!shader) return 0;

	auto instanced_version = shader->instanced_version.get();
	if (!instanced_version) return 0;

	while (i + 1 < commands.size()) {
		if (commands[i].key != commands[i + 1].key) break;

		count += 1;
		i += 1;
	}

	return count;
}

void switch_shader(World& world, RenderParams& params, ID shader_id, bool instanced) {
	auto shader = world.by_id<Shader>(shader_id);
	if (!shader) return;

	if (instanced) {
		auto instanced_shader = shader->instanced_version.get();
		if (!instanced_shader) return;

		shader = instanced_shader;
	}

	shader->bind();
	params.command_buffer.current_texture_index = 0;
	params.set_shader_scene_params(*shader, world);
}

void depth_func_bind(DepthFunc func) {
	switch (func) {
	case DepthFunc_Lequal: glDepthFunc(GL_LEQUAL); break;
	case DepthFunc_Less: glDepthFunc(GL_LESS); break;
	}
}

void cull_bind(Cull cull) {
	switch (cull) {
	case Cull_None: 
		glDisable(GL_CULL_FACE); 
		return;
	case Cull_Back: 
		glEnable(GL_CULL_FACE); 
		glCullFace(GL_BACK);
		return;
	case Cull_Front:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		return;
	}
}

void set_params(CommandBuffer& command_buffer, Material& mat, World& world) {
	auto& params = mat.params;
	auto shader = world.by_id<Shader>(mat.shader);
	if (!shader) return;

	auto previous_num_tex_index = command_buffer.current_texture_index;

	for (auto &param : params) {
		switch (param.type) {
		case Param_Vec3:
			param.loc.set_vec3(param.vec3);
			break;
		
		case Param_Vec2:
			param.loc.set_vec2(param.vec2);
			break;

		case Param_Mat4x4:
			param.loc.set_mat4(param.matrix); 
			break;

		case Param_Image: {
			auto tex = world.by_id<Texture>(param.image);
			if (!tex) continue;

			auto index = command_buffer.next_texture_index();
			tex->bind_to(index);
			param.loc.set_int(index);
			break;
		}
		case Param_Cubemap: {
			auto tex = world.by_id<Cubemap>(param.cubemap);
			if (!tex) continue;

			auto index = command_buffer.next_texture_index();


			tex->bind_to(index);
			param.loc.set_int(index);
			break;
		}
		case Param_Int:
			param.loc.set_int(param.integer);
			break;
		}
	}

	command_buffer.current_texture_index = previous_num_tex_index;
}

bool operator!=(Material& mat1, Material& mat2) {
	return std::memcmp(&mat1, &mat2, sizeof(Material)) != 0;
}

void CommandBuffer::submit_to_gpu(World& world, RenderParams& render_params) {
	//todo culling
	for (auto &cmd : commands) {
		cmd.key = ((long long)cmd.material->state->order << 36) 
			    + ((long long)cmd.material->state->depth_func << 32)
			    + (cmd.material->state->cull << 30) 
			    + (cmd.buffer->vao << 15) 
			    + (cmd.material->shader << 7) 
			    + ((long long)cmd.material % 64);
	}

	std::sort(commands.begin(), commands.end(), compare_key);
	bool last_was_instanced = false;

	for (int i = 0; i < commands.size();) {
		auto& cmd = commands[i];
		auto& mat = *cmd.material;

		auto num_instanceable = can_instance(commands, i, world);
		auto instanced = num_instanceable > 2;

		if (i == 0) {
			switch_shader(world, render_params, mat.shader, instanced);
			cmd.buffer->bind();
			depth_func_bind(mat.state->depth_func);
			cull_bind(mat.state->cull);
			if (mat.state->clear_depth_buffer) {
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			set_params(*this, mat, world);
		}
		else {
			auto& last_cmd = commands[i - 1];
			auto& last_mat = *last_cmd.material;

			if ((last_mat.shader != mat.shader) || last_was_instanced) {
				switch_shader(world, render_params, mat.shader, instanced);
			}

			if (last_cmd.buffer->vao != cmd.buffer->vao) {
				cmd.buffer->bind();
			}

			if (last_mat.state->cull != mat.state->cull) {
				cull_bind(mat.state->cull);
			}

			if (last_mat.state->depth_func != mat.state->depth_func) {
				depth_func_bind(last_mat.state->depth_func);
			}

			if (mat.state->clear_depth_buffer && !last_mat.state->clear_depth_buffer) {
				glClear(GL_DEPTH_BUFFER_BIT);
			}

			if ((mat != last_mat) || (last_was_instanced != instanced)) {
				set_params(*this, mat, world);
			}
		}

		if (instanced) {
			auto vao = cmd.buffer->vao;
			auto instance_buffer = this->instanced_buffers[cmd.key];

			if (instance_buffer == 0) {
				unsigned int buff;
				glGenBuffers(1, &buff);

				cmd.buffer->bind();

				glBindBuffer(GL_ARRAY_BUFFER, buff);
				glBufferData(GL_ARRAY_BUFFER, num_instanceable * sizeof(glm::mat4), (void*)0, GL_STREAM_DRAW);

				for (int attrib = 0; attrib < 4; attrib++) {
					glEnableVertexAttribArray(5 + attrib); //todo make this less hardcoded
					glVertexAttribPointer(5 + attrib, 4, GL_FLOAT, false, sizeof(glm::mat4), (void*)(attrib * 4 * sizeof(float)));
				}
			
				for (int attrib = 0; attrib < 4; attrib++) {
					glVertexAttribDivisor(5 + attrib, 1);
				}

				this->instanced_buffers[cmd.key] = buff;

				instance_buffer = buff;

				auto transforms = TEMPORARY_ARRAY(glm::mat4, num_instanceable);

				for (int c = 0; c < num_instanceable; c++) {
					transforms[i] = *commands[i].model_m;
					i += 1;
				}

				glBindBuffer(GL_ARRAY_BUFFER, instance_buffer);
				glBufferData(GL_ARRAY_BUFFER, num_instanceable * sizeof(glm::mat4), transforms, GL_STREAM_DRAW);
			}
		}
		else {
			auto shader = world.by_id<Shader>(mat.shader);
			if (shader == NULL) { continue; }

			shader->model.set_mat4(*cmd.model_m);
			glDrawElements(GL_TRIANGLES, cmd.buffer->length, GL_UNSIGNED_INT, NULL);
			i += 1;
		}
		last_was_instanced = instanced;
	}
}