#pragma once

#include <stb_image.h>
#include <string>
#include "reflection.h"

using TextureID = unsigned int;

struct Texture {
	std::string filename;
	TextureID texture_id = 0;

	void on_load(struct World&);
	void bind_to(unsigned int);

	REFLECT()
};

struct Cubemap {
	std::string filename;
	TextureID texture_id = 0;
	void bind_to(unsigned int);

	REFLECT()
};

Texture* load_Texture(struct World&, const std::string& filename);
Texture* make_Texture(struct World&);
Cubemap* make_Cubemap(struct World&);