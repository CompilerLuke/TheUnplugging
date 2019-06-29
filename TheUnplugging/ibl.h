#pragma once

#include "id.h"
#include "system.h"
#include <string>
#include "reflection.h"
#include "handle.h"

struct Skybox {
	std::string filename;

	Handle<struct Cubemap> env_cubemap;
	Handle<struct Cubemap> irradiance_cubemap;
	Handle<struct Cubemap> prefilter_cubemap;
	Handle<struct Texture> brdf_LUT;

	void on_load(struct World&);
	void set_ibl_params(Handle<struct Shader>, struct World&, RenderParams&);

	REFLECT()
};

Skybox* load_Skybox(struct World&, const std::string&);