#include "Animation.h"
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <Helpers/TimerHelper/TimerHelper.h>
#include <mutex>
#include <future>

std::mutex Animation::s_AnimMutex;

Animation::Animation(const std::string& name, const std::string& animationPath) { 
    SetName(name);
    m_AnimationPath = animationPath;

    Assimp::Importer importer; 
	m_Scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(m_Scene && m_Scene->mRootNode);
    
    auto anim = m_Scene->mAnimations[0];
    m_Duration = anim->mDuration;
    m_TicksPerSecond = anim->mTicksPerSecond;


    aiMatrix4x4 globalTransformation = m_Scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();

}

Bone *Animation::FindBone(const std::string &name) { 
    auto iter =
            std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone &Bone) { return Bone.GetBoneName() == name; });
    if (iter == m_Bones.end())
        return nullptr;
    else
        return &(*iter);
}

float Animation::GetTicksPerSecond() { return m_TicksPerSecond; }

float Animation::GetDuration() { return m_Duration; }

const AssimpNodeData &Animation::GetRootNode() { return m_RootNode; }

const std::map<std::string, BoneInfo> &Animation::GetBoneIDMap() { return m_BoneInfoMap; }

const std::vector<FlatBoneNode> &Animation::GetFlatBoneList() { return m_FlatBoneList; }

void Animation::SetName(const std::string &name) { m_Name = name; }

const std::string Animation::GetName() { return m_Name; }


void Animation::ProcessAnimation_Async(MeshInstance *mesh, AssimpNodeData *rootNode,
                                       std::map<std::string, BoneInfo> *boneIDMap,
                                       std::vector<FlatBoneNode> *flatBoneList, bool *ready) {
    std::lock_guard<std::mutex> lock(s_AnimMutex);

    if (!mesh->model->Ready)
    {
        if (mesh->model->Material.diffuse->path.empty())
        {
            EngineDebug::GetInstance().PrintError(mesh->model->Directory + " diffuse is not ready");
        }
        if (mesh->model->Material.specular->path.empty())
        {
            EngineDebug::GetInstance().PrintError(mesh->model->Directory + " specular is not ready");
        }
        EngineDebug::GetInstance().PrintError("Tried processing animation, but model isnt ready!");
    }

    Assimp::Importer importer;
    const aiScene* m_Scene = importer.ReadFile(m_AnimationPath, aiProcess_Triangulate);
    assert(m_Scene && m_Scene->mRootNode);
    
    m_BoneInfoMap.clear();
    m_Bones.clear();
    m_FlatBoneList.clear();

    readHierarchyData(m_RootNode, m_Scene->mRootNode);

    readMissingBones(m_Scene->mAnimations[0], *mesh);

    flattenHierarchy(&m_RootNode, -1, m_BoneInfoMap, m_FlatBoneList);
    EngineDebug::GetInstance().PrintInfo("Done processing animation " + m_Name);
    Ready = true;
}

const std::string &Animation::GetAnimationPath() { 
    return m_AnimationPath; 
}

void Animation::ProcessAnimation(MeshInstance *mesh) {
    TimerHelper timer("Animation::ProcessAnimation::");
   
    
    //m_Futures.push_back(std::async(std::launch::async, &Animation::ProcessAnimation_Async, this, mesh, &m_RootNode, &m_BoneInfoMap, &m_FlatBoneList, &Ready));

    Assimp::Importer importer;
    const aiScene* m_Scene = importer.ReadFile(m_AnimationPath, aiProcess_Triangulate);
    assert(m_Scene && m_Scene->mRootNode);

    m_BoneInfoMap.clear();
    m_Bones.clear();
    m_FlatBoneList.clear();

    readHierarchyData(m_RootNode, m_Scene->mRootNode);

    readMissingBones(m_Scene->mAnimations[0], *mesh);

    flattenHierarchy(&m_RootNode, -1, m_BoneInfoMap, m_FlatBoneList);
    EngineDebug::GetInstance().PrintInfo("Done processing animation " + m_Name);
    Ready = true;

}

// private

void Animation::readMissingBones(const aiAnimation* animation, MeshInstance& mesh) {
    int size = animation->mNumChannels;

    auto &boneInfoMap = mesh.GetBoneInfoMap();
    int &boneCount = mesh.GetBoneCount();
    m_BoneMap.clear();

    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        Bone bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel);
        m_Bones.push_back(bone);
        m_BoneMap[bone.GetBoneName()] = m_Bones.size() - 1;

    }
    m_BoneInfoMap = boneInfoMap;
}

void Animation::readHierarchyData(AssimpNodeData &dest, const aiNode *src) { 
    assert(src);
    
    dest.name = src->mName.data;
    dest.transformation = Util::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        readHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

void Animation::flattenHierarchy(const AssimpNodeData *node, int parentIndex,
                                 const std::map<std::string, BoneInfo> &boneInfoMap,
                                std::vector<FlatBoneNode> &flatList) {
    if (isJunkBone(node->name) && boneInfoMap.find(node->name) == boneInfoMap.end()) {
        return; // pomi� w�z�y ko�cowe, kt�re nie maj� kana�u
    }

     FlatBoneNode flatNode;
    flatNode.name = node->name;
    flatNode.parentIndex = parentIndex;
    flatNode.localBindTransform = node->transformation;
    flatNode.bone = FindBone(node->name); // nullptr je�li nie ma bone'a

    auto it = boneInfoMap.find(node->name);
    flatNode.offset = (it != boneInfoMap.end()) ? it->second.offset : glm::mat4(1.0f);

    int currentIndex = static_cast<int>(flatList.size());
    flatList.push_back(flatNode);

    for (int i = 0; i < node->childrenCount; ++i) {
        flattenHierarchy(&node->children[i], currentIndex, boneInfoMap, flatList);
    }
}

bool Animation::isJunkBone(const std::string &boneName) {
    return boneName.ends_with("_end") || boneName.ends_with("_End") || boneName.ends_with("_end_end");
}
