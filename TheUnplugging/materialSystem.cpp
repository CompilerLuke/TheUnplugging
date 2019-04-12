#include "materialSystem.h"

Material* material_by_name(std::vector<Material> materials, const std::string& name) {
	for (auto &mat : materials) {
		if (mat.name == name) return &mat;
	}
	return NULL;
}