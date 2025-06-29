#include "Model.h"
#include "ResourceManager/ResourceManager.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/config.h>
#include <Helpers/TimerHelper/TimerHelper.h>
#include "BoundingVolume/AABB.h"

Model Model::LoadModel(std::string_view path) 
{ 

	Assimp::Importer import;
    import.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_TEXTURES, true);
    const aiScene *scene =
            import.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return Model();
    }
    std::cout << "Model loaded successfully!" << std::endl;

    Model model;

    model.Name = path.substr(path.find_last_of('/'), path.size());
    model.Directory = path.substr(0, path.find_last_of('/'));
    model.processNode(scene->mRootNode, scene);
    model.GenerateAABB();

    return model;

}

void Model::processNode(aiNode *node, const aiScene *scene) 
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        Meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) 
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        setVertexBoneDataToDefault(vertex);
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        vertex.tangent = glm::vec3(0);
        vertex.bitangent = glm::vec3(0);

        if (mesh->HasNormals()) {
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        if (mesh->HasTangentsAndBitangents()) {
            // These are already generated because of aiProcess_CalcTangentSpace
            vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            vertex.bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        } else {
            vertex.tangent = glm::vec3(0.0f);
            vertex.bitangent = glm::vec3(0.0f);
        }

        // check if mesh contains any textures
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we
            // won't use models where a vertex can have multiple texture coordinates so we always take the first
            // set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;

        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(std::move(vertex));
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // process materials
    aiMaterial *aiMaterial = std::move(scene->mMaterials[mesh->mMaterialIndex]);


    // 1. diffuse maps
    std::vector<std::shared_ptr<Texture>> diffuseMaps =
            loadMaterialTextures(aiMaterial, aiTextureType_DIFFUSE, "texture_diffuse", scene);
    // textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    if (diffuseMaps.size() > 0)
        Material.diffuse = std::move(diffuseMaps[0]);

    // 2. specular maps
    std::vector<std::shared_ptr<Texture>> specularMaps =
            loadMaterialTextures(aiMaterial, aiTextureType_SPECULAR, "texture_specular", scene);
    // textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    if (specularMaps.size() > 0)
        Material.specular = std::move(specularMaps[0]);


    // 3. normal maps
    std::vector<std::shared_ptr<Texture>> normalMaps =
            loadMaterialTextures(aiMaterial, aiTextureType_HEIGHT, "texture_normal", scene);
    // textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    if (normalMaps.size() > 0)
        Material.normal = normalMaps[0];

    // 4. height maps
    std::vector<std::shared_ptr<Texture>> heightMaps =
            loadMaterialTextures(aiMaterial, aiTextureType_DISPLACEMENT, "texture_height", scene);
    // textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    if (heightMaps.size() > 0)
        Material.height = std::move(heightMaps[0]);

    // 5.Roughness maps
    std::vector<std::shared_ptr<Texture>> roughnessMaps =
            loadMaterialTextures(aiMaterial, aiTextureType_SHININESS, "texture_roughness", scene);
    // textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    if (roughnessMaps.size() > 0)
        Material.roughness = std::move(roughnessMaps[0]);


    // 6.Metallic maps
    std::vector<std::shared_ptr<Texture>> metallicMaps =
            loadMaterialTextures(aiMaterial, aiTextureType_METALNESS, "texture_metallic", scene);
    // textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
    if (metallicMaps.size() > 0)
        Material.metallic = std::move(metallicMaps[0]);

    // 7. ambient occlusion maps
    std::vector<std::shared_ptr<Texture>> ambientOcclusionMaps =
            loadMaterialTextures(aiMaterial, aiTextureType_AMBIENT, "texture_ambient_occlusion", scene);
    // textures.insert(textures.end(), ambientOcclusionMaps.begin(), ambientOcclusionMaps.end());

    if (ambientOcclusionMaps.size() > 0)
        Material.ambientOcclusion = ambientOcclusionMaps[0];

    if (Material.IsEmpty()) {
    }

    extractBoneWeightForVertices(vertices, mesh, scene);

    return Mesh(std::move(vertices), std::move(indices));
}


std::shared_ptr<Texture> Model::loadEmbededTexture(const aiTexture *pAiTexture) 
{
    std::shared_ptr<Texture> texture;

    if (pAiTexture->mHeight == 0) {
        std::vector<unsigned char> textureData(reinterpret_cast<unsigned char*>(pAiTexture->pcData),
            reinterpret_cast<unsigned char*>(pAiTexture->pcData) + pAiTexture->mWidth);

        texture = ResourceManager::GetInstance()
                            .LoadTextureFromFile(pAiTexture->mFilename.C_Str(), true, textureData, pAiTexture->mWidth);
    }
    return texture;
}

void Model::setVertexBoneDataToDefault(Vertex &vertex) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void Model::setVertexBoneData(Vertex &vertex, int boneID, float weight) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_BoneIDs[i] < 0) {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            break;
        }
    }
}

void Model::extractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh, const aiScene *scene) 
{
    auto &boneInfoMap = BoneInfoMap;
    int &boneCount = m_BoneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = Util::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        } else {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= vertices.size());
            setVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

std::vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                                                  std::string typeName,
                                          const aiScene *scene) {
    std::vector<std::shared_ptr<Texture>> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string path = str.C_Str();

        const aiTexture *pAiTexture = scene->GetEmbeddedTexture(path.c_str());

        if (pAiTexture) {
            std::vector<unsigned char> textureData(reinterpret_cast<unsigned char*>(pAiTexture->pcData),
                reinterpret_cast<unsigned char*>(pAiTexture->pcData) + pAiTexture->mWidth);

            auto texture = ResourceManager::GetInstance()
                .LoadTextureFromFile(pAiTexture->mFilename.C_Str(), true,
                    textureData,
                    pAiTexture->mWidth);
            texture->type = typeName;

            textures.push_back(texture);
        } else {

            bool skip = false;
            /*for (unsigned int j = 0; j < textureLoaded.size(); j++) {
                if (std::strcmp(textureLoaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textureLoaded[j]);
                    skip = true;
                    break;
                }
            }*/
            if (!skip) { // if texture hasn't been loaded already, load it
                std::string filepath = Directory + '/' + path;
                auto texture = ResourceManager::GetInstance().LoadTextureFromFile(filepath.c_str());
                //texture.id = textureFromFile(str.C_Str(), Directory);
                texture->type = typeName;
                textures.push_back(texture);
            }
        }
    }
    return textures;
}


void Model::GenerateAABB()
{
    this->pAABB = std::make_unique<AABB>(AABB::GenerateAABB(Meshes));
}