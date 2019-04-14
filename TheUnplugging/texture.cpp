#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "ecs.h"
#include "glad/glad.h"

void Texture::on_load(World& world) {
	auto real_filename = world.level.asset_path(filename);

	stbi_set_flip_vertically_on_load(true);

	int width = 0;
	int height = 0;
	int nr_channels = 0;
	unsigned int texture_id = 0;

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);

	auto data = stbi_load(real_filename.c_str(), &width, &height, &nr_channels, 3);
	if (!data) throw "Could not load texture";

	auto internal_color_format = GL_RGB;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, internal_color_format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	this->texture_id = texture_id;
}

Texture* make_Texture(World& world) {
	ID id = world.make_ID();
	auto e = world.make<Entity>(id);
	e->layermask = game_layer;
	auto texture = world.make<Texture>(id);
	return texture;
}

void Texture::bind_to(unsigned int num) {
	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

void Cubemap::bind_to(unsigned int num) {
	glActiveTexture(GL_TEXTURE0 + num);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

Texture* load_Texture(World& world, const std::string& filename) {
	for (auto texture : world.filter<Texture>(any_layer)) {
		if (texture->filename == filename) return texture;
	}

	ID id = world.make_ID();
	auto e = world.make<Entity>(id);
	auto texture = world.make<Texture>(id);
	texture->filename = filename;
	texture->on_load(world);
	return texture;
}
