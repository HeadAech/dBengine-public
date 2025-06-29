
#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <map>
#include <string>
#include <string_view>
#include "Mesh.h"

#include "assimp/scene.h"
#include "Components/Animator/Bone/Bone.h"
#include "Material/Material.h"
#include "BoundingVolume/AABB.h"

#include <memory>

class Model
{
    

    public:
        std::string Name;    

        int m_BoneCounter = 0;
        Model() = default;

        ~Model() = default;

        bool Ready = false;

        std::vector<Mesh> Meshes;
        std::map<std::string, BoneInfo> BoneInfoMap;
        Material Material;

        std::string Directory;
        
        std::shared_ptr<AABB> pAABB;

        static Model LoadModel(std::string_view path);

        void processNode(aiNode *node, const aiScene *scene);
        

        Mesh processMesh(aiMesh *mesh, const aiScene *scene);

        std::shared_ptr<Texture> loadEmbededTexture(const aiTexture *pAiTexture);
        

        void setVertexBoneDataToDefault(Vertex &vertex);
        

        void setVertexBoneData(Vertex &vertex, int boneID, float weight);

        void extractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh, const aiScene *scene);

        std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                                                   std::string typeName,
                                                         const aiScene *scene);

        void GenerateAABB();
};

#endif // !MODEL_H
