#pragma once

#include "shaderManager.h"
#include "textureManager.h"
#include "modelManager.h"

namespace RHI {
	extern ShaderManager shader_manager;
	extern TextureManager texture_manager;
	extern CubemapManager cubemap_manager;
	extern ModelManager model_manager;
};