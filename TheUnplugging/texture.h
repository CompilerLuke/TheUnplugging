#pragma once

#include <stb_image.h>
#include <string>

using TextureID = unsigned int;

struct Texture {
	std::string filename;
	TextureID texture_id = 0;

	void on_load(struct World&);
	void bind_to(unsigned int);
};

struct Cubemap {
	TextureID id = 0;
	void bind_to(unsigned int);
};

Texture* load_Texture(struct World&, const std::string& filename);
