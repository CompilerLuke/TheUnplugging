#pragma once

#include "id.h"
#include "system.h"
#include <string>

struct Skybox {
	std::string filename;

	ID env_cubemap;
	ID irradiance_cubemap;
	ID prefilter_cubemap;
	ID brdf_LUT;

	void on_load(struct World&);
	void set_ibl_params(struct Shader&, struct World&, RenderParams&);
};

Skybox* load_Skybox(struct World&, const std::string&);