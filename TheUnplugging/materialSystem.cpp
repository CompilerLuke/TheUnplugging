#include "materialSystem.h"

Material* material_by_name(std::vector<Material>& materials, const std::string& name) {
	for (int i = 0; i < materials.size(); i++) {
		if (materials[i].name == name) return &materials[i];
	}
	return NULL;
}

Param::Param() {};

Param make_Param_Vec3(Uniform loc, glm::vec3 vec) {
	Param param;
	param.loc = loc;
	param.type = Param_Vec3;
	param.vec3 = vec;
	return param;
}

Param make_Param_Cubemap(Uniform loc, ID id) {
	Param param;
	param.loc = loc;
	param.type = Param_Cubemap;
	param.cubemap = id;
	return param;
}