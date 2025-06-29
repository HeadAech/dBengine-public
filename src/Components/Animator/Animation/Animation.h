
#include <vector>
#include <string>
#ifndef ANIMATION_H
#define ANIMATION_H

#include "glm/glm.hpp"
#include <Components/MeshInstance/MeshInstance.h>
#include <Components/Animator/Bone/Bone.h>
#include <assimp/Importer.hpp>
#include <future>

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

struct FlatBoneNode {
    std::string name;
    int parentIndex;
    glm::mat4 localBindTransform;
    glm::mat4 offset;
    Bone *bone;
};

struct FlatBoneNodeBlended {
    std::string name;
    int parentIndex;
    glm::mat4 baseBindTransform;
    glm::mat4 layeredBindTransform;
    glm::mat4 offset;
    Bone *baseBone = nullptr;
    Bone *layeredBone = nullptr;
};

class Animation {
    public:
        Animation() = default;

        Animation(const std::string& name, const std::string &animationPath);
        ~Animation() = default;

        Bone *FindBone(const std::string &name);

        float GetTicksPerSecond();
        float GetDuration();
        const AssimpNodeData &GetRootNode();
        const std::map<std::string, BoneInfo> &GetBoneIDMap();

        const std::vector<FlatBoneNode> &GetFlatBoneList();

        void SetName(const std::string &name);
        const std::string GetName();

        void ProcessAnimation(MeshInstance *mesh);
        void ProcessAnimation_Async(MeshInstance *mesh, AssimpNodeData *rootNode,
                                    std::map<std::string, BoneInfo> *boneIDMap,
                                    std::vector<FlatBoneNode> *flatBoneList, bool* ready);
        const std::string &GetAnimationPath();

        bool Ready = false;

        static std::mutex s_AnimMutex;

    private:
        
        std::vector<std::future<void>> m_Futures;

        std::string m_AnimationPath;

        const aiScene* m_Scene;

        aiAnimation m_Anim;

        void readMissingBones(const aiAnimation *animation, MeshInstance &mesh);

        void readHierarchyData(AssimpNodeData &dest, const aiNode *src);

        void flattenHierarchy(const AssimpNodeData *node, int parentIndex,
                             const std::map<std::string, BoneInfo> &boneInfoMap,
                             std::vector<FlatBoneNode> &flatList);
        
        bool isJunkBone(const std::string &boneName);

        std::string m_Name;

        float m_Duration;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
        std::unordered_map<std::string, int> m_BoneMap;

        std::vector<FlatBoneNode> m_FlatBoneList;
};

#endif // !ANIMATION_H
