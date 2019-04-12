#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/vector3.h>
#include <assimp/vector2.h>
#include <assimp/postprocess.h>

Mesh process_mesh(aiMesh* mesh, const aiScene* scene, std::vector<std::string>& materials) {
	auto vertices = std::vector<Vertex>();
	vertices.reserve(mesh->mNumVertices);

	auto indices = std::vector<unsigned int>();
	
	AABB aabb;
	
	for (int i = 0; i < mesh->mNumVertices; i++) {
		auto position = mesh->mVertices[i];

		aabb.update(glm::vec3(position.x, position.y, position.z));

		auto tangent = mesh->mTangents[i];
		auto bitangent = mesh->mBitangents[i];

		auto coords = mesh->mTextureCoords[0];
		if (!coords) throw "Mesh does't have uvs";

		auto first_coords = coords[i];
		auto normals = mesh->mNormals[i];

		Vertex v = {
			glm::vec3(position.x, position.y, position.z),
			glm::vec3(normals.x, normals.y, normals.z),
			glm::vec2(first_coords.x, first_coords.y),
			glm::vec3(tangent.x, tangent.y, tangent.z),
			glm::vec3(bitangent.x, bitangent.y, bitangent.z)
		};

		vertices.push_back(v);
	}

	int indices_count = 0;
	for (int i = 0; i < mesh->mNumFaces; i++) {
		indices_count += mesh->mFaces[i].mNumIndices;
	}

	indices.reserve(indices_count);
	for (int i = 0; i < mesh->mNumFaces; i++) {
		auto face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	auto aMat = scene->mMaterials[mesh->mMaterialIndex];
	aiString c_name;
	aiGetMaterialString(aMat, AI_MATKEY_NAME, &c_name);

	std::string name(c_name.data);

	int id = -1;
	for (int i = 0; i < materials.size(); i++) {
		if (materials[i] == name) {
			id = i;
		}
	}

	if (id == -1) {
		materials.push_back(name);
		id = materials.size() - 1;
	}

	Mesh new_mesh;
	new_mesh.vertices = std::move(vertices);
	new_mesh.indices = std::move(indices);
	new_mesh.aabb = std::move(aabb);
	new_mesh.material_id = id;
	return new_mesh;
}

void process_node(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, std::vector<std::string>& materials) {
	meshes.reserve(meshes.size() + node->mNumMeshes);

	for (int i = 0; i < node->mNumMeshes; i++) {
		auto mesh_id = node->mMeshes[i];
		auto mesh = scene->mMeshes[mesh_id];
		meshes.push_back(process_mesh(mesh, scene, materials));
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		process_node(node->mChildren[i], scene, meshes, materials);
	}
}

void Model::load_in_place(World& world) {
	auto real_path = world.level.asset_path(this->path);
	
	Assimp::Importer importer;
	auto scene = importer.ReadFile(real_path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene) throw std::string("Could not load model ") + path + std::string(" ") + real_path;
	
	std::vector<Mesh> meshes;
	std::vector<std::string> materials;

	process_node(scene->mRootNode, scene, meshes, materials);

	this->meshes = std::move(meshes);
	this->materials = std::move(materials);
}

Model* load_Model(World& world, const std::string& path) {
	for (auto existing_model : world.filter<Model>(any_layer)) {
		if (existing_model->path == path) return existing_model;
	}

	ID id = world.make_ID();
	auto entity = world.make<Entity>(id);
	auto model = world.make<Model>(id);
	model->path = path;
	model->load_in_place(world);
	return model;
}

