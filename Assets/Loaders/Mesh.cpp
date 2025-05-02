#include "Mesh.h"
#include "../Asset.h"
#include "../../debug_panic.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "assimp/scene.h"
#include "ProjectManager/projectManager.h"


MemMesh* MemMesh::loadMeshfromAsset(love::Asset* asset){
    static Assimp::Importer Importer;
    if (asset->relative_path.begin() == asset->relative_path.end()) {
        panic("no path to load");
    }
    auto first = *asset->relative_path.begin();

    if (first==":mem:"||first==":pak:"||first==":net:") panic("not implemented");
    if (asset->relative_path.empty())panic("no path to load");
	auto a = Importer.ReadFile((char*)((love::project::loaded_project_path/asset->relative_path).u8string().c_str()),aiPostProcessSteps::aiProcess_Triangulate|aiProcess_CalcTangentSpace|aiProcess_GenSmoothNormals);
	if (a->hasSkeletons()) panic("not implemented");
    if (a->HasTextures()) panic("not implemented");
    const auto dat = new MemMesh();
    for (int i =0;i< a->mNumMeshes;i++) {
        dat->mesh_i_offsets.push_back(dat->indices.size());
        dat->mesh_v_offsets.push_back(dat->vertices.size());
        auto mesh = a->mMeshes[i];
        for (int j = 0; j < mesh->mNumFaces; j++) {
            auto f = mesh->mFaces[j];
            for (int k = 0; k < 3; k++) {
                dat->indices.push_back(f.mIndices[k]);
            }
        }
        dat->vertices.insert(dat->vertices.end(),(glm::vec3*)(mesh->mVertices),(glm::vec3*)(mesh->mVertices+mesh->mNumVertices));
        dat->uvs.insert(dat->uvs.end(),(glm::vec3*)mesh->mTextureCoords[0],(glm::vec3*)(mesh->mTextureCoords[0]+mesh->mNumVertices));
        dat->tangents.insert(dat->tangents.end(),(glm::vec3*)mesh->mTangents,(glm::vec3*)((glm::vec3*)mesh->mTangents+mesh->mNumVertices));
        dat->normals.insert(dat->normals.end(),(glm::vec3*)mesh->mNormals,(glm::vec3*)(mesh->mNormals+mesh->mNumVertices));

    }

    return dat;
	return 0;
  }
