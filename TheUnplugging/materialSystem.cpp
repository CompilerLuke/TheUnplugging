#include "materialSystem.h"

REFLECT_UNION_BEGIN(Param)
REFLECT_UNION_FIELD(loc)
REFLECT_UNION_CASE(vec3)
REFLECT_UNION_CASE(vec2)
REFLECT_UNION_CASE(matrix)
REFLECT_UNION_CASE(image)
REFLECT_UNION_CASE(cubemap)
REFLECT_UNION_CASE(integer)
REFLECT_UNION_END()

REFLECT_STRUCT_BEGIN(Material)
REFLECT_STRUCT_MEMBER(name)
REFLECT_STRUCT_MEMBER(shader)
REFLECT_STRUCT_MEMBER(params)
REFLECT_STRUCT_MEMBER(state)
REFLECT_STRUCT_END()

Material* material_by_name(std::vector<Material>& materials, const std::string& name) {
	for (int i = 0; i < materials.size(); i++) {
		if (materials[i].name == name) return &materials[i];
	}
	return NULL;
}

Param::Param() {};

Param make_Param_Vec3(const Uniform& loc, glm::vec3 vec) {
	Param param;
	param.loc = loc;
	param.type = Param_Vec3;
	param.vec3 = vec;
	return param;
}

Param make_Param_Cubemap(const Uniform& loc, ID id) {
	Param param;
	param.loc = loc;
	param.type = Param_Cubemap;
	param.cubemap = id;
	return param;
}