#include "materialSystem.h"
#include "texture.h"

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
REFLECT_STRUCT_MEMBER_TAG(shader, reflect::ShaderIDTag)
REFLECT_STRUCT_MEMBER(params)
REFLECT_STRUCT_MEMBER(state)
REFLECT_STRUCT_END()

Material* material_by_name(vector<Material>& materials, const std::string& name) {
	for (int i = 0; i < materials.length; i++) {
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

Param make_Param_Vec2(const Uniform& loc, glm::vec2 vec) {
	Param param;
	param.loc = loc;
	param.type = Param_Vec2;
	param.vec2 = vec;
	return param;
}

Param make_Param_Cubemap(const Uniform& loc, ID id) {
	Param param;
	param.loc = loc;
	param.type = Param_Cubemap;
	param.cubemap = id;
	return param;
}

Param make_Param_Image(const Uniform& loc, ID id) {
	Param param;
	param.loc = loc;
	param.type = Param_Image;
	param.image = id;
	return param;
}

vector<Param> make_SubstanceMaterial(World& world, const std::string& folder, const std::string& name) {
	auto shad = load_Shader(world, "shaders/pbr.vert", "shaders/pbr.frag");
	
	return {
		make_Param_Image(shad->location("material.diffuse"), world.id_of(load_Texture(world, folder + "\\" + name + "_basecolor.jpg"))),
		make_Param_Image(shad->location("material.metallic"), world.id_of(load_Texture(world, folder + "\\" + name + "_metallic.jpg"))),
		make_Param_Image(shad->location("material.roughness"), world.id_of(load_Texture(world, folder + "\\" + name + "_roughness.jpg"))),
		make_Param_Image(shad->location("material.normal"), world.id_of(load_Texture(world, folder + "\\" + name + "_normal.jpg"))),
		make_Param_Vec2(shad->location("transformUVs"), glm::vec2(100, 100))
	};
}