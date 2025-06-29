#include "Animator.h"
#include <Signal/Signals.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <future>
#include <glm/gtx/matrix_decompose.hpp>
#include <mutex>
#include "Animation/Animation.h"

Animator::Animator() {
    name = "Animator";
    icon = ICON_FA_BLIND;

    m_CurrentTime = 0.0f;

    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++) {
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    Signals::AnimationLibrary_AnimationDeleted.connect(this->GetUUID(), [this](std::shared_ptr<Animation> pAnim) {
        auto it = std::find(m_Animations.begin(), m_Animations.end(), pAnim);

        if (it != m_Animations.end()) {
            DeselectAnimation();
            m_Animations.erase(it);
        }
    });
}

Animator::~Animator() { Signals::AnimationLibrary_AnimationDeleted.disconnect(this->GetUUID()); }

void Animator::Update(float deltaTime) {
    if (!enabled)
        return;

    for (auto anim: m_Animations) {
        if (!anim->Ready) {
            if (auto mesh = gameObject->GetComponent<MeshInstance>()) {
                if (mesh->model->Ready) {
                    anim->ProcessAnimation(mesh);
                }
            }
        }
    }

    m_DeltaTime = deltaTime;

    if (m_CurrentAnimation && m_Playing && m_CurrentAnimation->Ready) {
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * deltaTime * m_TimeScale;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());


        if (m_TransitionTime > 0.0f) {
            m_TransitionTime -= deltaTime;

            float blendFactor = 1.0f - (m_TransitionTime / m_TransitionDuration);
            blendFactor = glm::clamp(blendFactor, 0.0f, 1.0f);

            m_BlendFactor = blendFactor;

        } else {
            // m_Blending = false;
            // m_CurrentAnimation = m_CurrentLayeredAnimation;
            // m_CurrentLayeredAnimation = nullptr;
            m_BlendFactor = 1.0f;

            if (m_Blending)
            {
                m_Blending = false;
                m_CurrentAnimation = m_CurrentLayeredAnimation;
                m_CurrentTime = timeLayer;
                m_CurrentLayeredAnimation = nullptr;
            }
        }

        if (m_CurrentLayeredAnimation && m_CurrentLayeredAnimation->Ready)
            BlendTwoAnimations(m_CurrentAnimation.get(), m_CurrentLayeredAnimation.get(), m_BlendFactor, deltaTime * m_TimeScale);
        else if (m_CurrentAnimation)
            EvaluateFlatBoneHierarchy(m_CurrentAnimation->GetFlatBoneList());
    }
}

void Animator::PlayAnimation(const std::string &animName) {
    auto animation = GetAnimation(animName);

    if (animation == nullptr) {
        EngineDebug::GetInstance().PrintError("Animation " + animName + " not found in animator.");
        return;
    }

    m_CurrentAnimation = animation;
    m_CurrentTime = 0.0f;
    m_Playing = true;
}

void Animator::CalculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform,
                                      std::map<std::string, BoneInfo> boneInfoMap) {

    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone *bone = m_CurrentAnimation->FindBone(nodeName);

    if (bone) {
        bone->Update(m_CurrentTime);
        nodeTransform = bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;


    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++) {
        CalculateBoneTransform(&node->children[i], globalTransformation, boneInfoMap);
    }
}

void Animator::EvaluateFlatBoneHierarchy(const std::vector<FlatBoneNode> &flatNodes) {
    std::lock_guard<std::mutex> lock(m_CurrentAnimation->s_AnimMutex);

    std::vector<glm::mat4> globalTransforms(flatNodes.size());

    for (size_t i = 0; i < flatNodes.size(); ++i) {
        const auto &node = flatNodes[i];

        glm::mat4 localTransform = node.localBindTransform;

        if (node.bone) {
            node.bone->Update(m_CurrentTime);
            localTransform = node.bone->GetLocalTransform();
        }

        glm::mat4 parentTransform = (node.parentIndex >= 0) ? globalTransforms[node.parentIndex] : glm::mat4(1.0f);
        glm::mat4 globalTransform = parentTransform * localTransform;

        globalTransforms[i] = globalTransform;

        // Zapisz do finalnych macierzy, je�li ta ko�� by�a w BoneMap
        if (node.bone) {
            int id = m_CurrentAnimation->GetBoneIDMap().at(node.name).id;
            if (id < m_FinalBoneMatrices.size())
                m_FinalBoneMatrices[id] = globalTransform * node.offset;
        }
    }
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices() { return m_FinalBoneMatrices; }

bool Animator::IsPlaying() { return m_Playing; }

void Animator::Pause() { m_Playing = false; }

void Animator::Play() {

    if (m_CurrentAnimation) {
        m_Playing = true;
    }
}

void Animator::Reset() {
    m_Playing = false;
    m_CurrentTime = 0.0f;
}

void Animator::AddAnimation(std::shared_ptr<Animation> &animation) {
    auto it = std::find(m_Animations.begin(), m_Animations.end(), animation);
    if (it != m_Animations.end()) {
        EngineDebug::GetInstance().PrintInfo("Can't add animation " + animation->GetName() +
                                             " to animator - Animation already exists.");
        return;
    }

    m_Animations.push_back(animation);
    m_Animations.back()->Ready = false;
    // m_Animations.back()->ProcessAnimation(gameObject->GetComponent<MeshInstance>());
}

void Animator::RemoveAnimation(std::shared_ptr<Animation> animation) {

    auto it = std::find(m_Animations.begin(), m_Animations.end(), animation);
    if (it != m_Animations.end()) {
        m_Animations.erase(it);
    } else {
        EngineDebug::GetInstance().PrintInfo("Can't remove animation " + animation->GetName() +
                                             " from animator - Animation not found.");
    }
}

std::shared_ptr<Animation> Animator::GetAnimation(const std::string &animName) {

    auto it = std::find_if(m_Animations.begin(), m_Animations.end(),
                           [&](std::shared_ptr<Animation> animation) { return animation->GetName() == animName; });

    if (it == m_Animations.end()) {
        return nullptr;
    }

    return *it;
}

Animation *Animator::GetCurrentAnimation() { return m_CurrentAnimation.get(); }

float Animator::GetCurrentPlaybackTime() { return m_CurrentTime; }

void Animator::SetCurrentPlaybackTime(float time) {

    if (m_CurrentAnimation) {
        m_CurrentTime = time;
    }
}

std::vector<std::string> Animator::GetAllAnimationsNames() const {
    std::vector<std::string> names;

    for (auto pAnim: m_Animations) {
        names.push_back(pAnim->GetName());
    }

    return names;
}

int Animator::GetIndexOfCurrentAnimation() const {
    int index = -1;

    for (int i = 0; i < m_Animations.size(); i++) {
        if (m_Animations[i] == m_CurrentAnimation) {
            index = i;
        }
    }

    return index;
}

void Animator::SetCurrentAnimation(int index) {
    if (index < m_Animations.size()) {
        m_CurrentAnimation = m_Animations.at(index);
    }
}

void Animator::SetCurrentAnimation(std::string name) {
    for (const auto &animation: m_Animations) {
        if (animation->GetName() == name) {
            m_CurrentAnimation = animation;
            return;
        }
    }
    // Optional: Handle case where animation is not found
    // e.g., log warning or set default animation
}

void Animator::DeselectAnimation() {
    m_Playing = false;
    m_CurrentAnimation = nullptr;
    m_CurrentTime = 0.0f;
}

float Animator::GetTimeScale() const { return m_TimeScale; }

void Animator::SetTimeScale(float timeScale) { m_TimeScale = timeScale; }


void Animator::BlendTwoAnimations(Animation *pBase, Animation *pLayer, float blendFactor, float deltaTime) {


    {
        std::lock_guard<std::mutex> lock(pBase->s_AnimMutex);
        float speedBase = (1.0f - blendFactor) + blendFactor * (pBase->GetDuration() / pLayer->GetDuration());
        timeBase += pBase->GetTicksPerSecond() * deltaTime * speedBase;
        timeBase = fmod(timeBase, pBase->GetDuration());
    }


    {
        std::lock_guard<std::mutex> lock(pLayer->s_AnimMutex);
        float speedLayer = (1.0f - blendFactor) * (pLayer->GetDuration() / pBase->GetDuration()) + blendFactor;
        timeLayer += pLayer->GetTicksPerSecond() * deltaTime * speedLayer;
        timeLayer = fmod(timeLayer, pLayer->GetDuration());
    }


    EvaluateBlendedFlatBoneHierarchy(m_FlatBlendedNodes, timeBase, timeLayer, blendFactor);
}

void Animator::CalculateBlendedBoneTransform(Animation *pAnimationBase, const AssimpNodeData *node,
                                             Animation *pAnimationLayer, const AssimpNodeData *nodeLayered,
                                             const float currentTimeBase, const float currentTimeLayered,
                                             const glm::mat4 &parentTransform, const float blendFactor) {
    const std::string &nodeName = node->name;

    glm::mat4 nodeTransform = node->transformation;
    Bone *pBone = pAnimationBase->FindBone(nodeName);
    if (pBone) {
        pBone->Update(currentTimeBase);
        nodeTransform = pBone->GetLocalTransform();
    }

    glm::mat4 layeredNodeTransform = nodeLayered->transformation;
    pBone = pAnimationLayer->FindBone(nodeName);
    if (pBone) {
        pBone->Update(currentTimeLayered);
        layeredNodeTransform = pBone->GetLocalTransform();
    }

    // Blend two matrices
    const glm::quat rot0 = glm::quat_cast(nodeTransform);
    const glm::quat rot1 = glm::quat_cast(layeredNodeTransform);
    const glm::quat finalRot = glm::slerp(rot0, rot1, blendFactor);
    glm::mat4 blendedMat = glm::mat4_cast(finalRot);
    blendedMat[3] = (1.0f - blendFactor) * nodeTransform[3] + layeredNodeTransform[3] * blendFactor;

    glm::mat4 globalTransformation = parentTransform * blendedMat;

    const auto &boneInfoMap = pAnimationBase->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        const int index = boneInfoMap.at(nodeName).id;
        const glm::mat4 &offset = boneInfoMap.at(nodeName).offset;

        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (size_t i = 0; i < node->children.size(); ++i) {
        if (nodeLayered->children.size() <= i) {
            break;
        }
        CalculateBlendedBoneTransform(pAnimationBase, &node->children[i], pAnimationLayer, &nodeLayered->children[i],
                                      currentTimeBase, currentTimeLayered, globalTransformation, blendFactor);
    }
}

void Animator::EvaluateBlendedFlatBoneHierarchy(const std::vector<FlatBoneNodeBlended> &flatNodes,
                                                float currentTimeBase, float currentTimeLayered, float blendFactor) {
    std::vector<glm::mat4> globalTransforms(flatNodes.size());

    for (size_t i = 0; i < flatNodes.size(); ++i) {
        const auto &node = flatNodes[i];

        glm::mat4 transformBase = node.baseBindTransform;
        if (node.baseBone) {
            node.baseBone->Update(currentTimeBase);
            transformBase = node.baseBone->GetLocalTransform();
        }

        glm::mat4 transformLayer = node.layeredBindTransform;
        if (node.layeredBone) {
            node.layeredBone->Update(currentTimeLayered);
            transformLayer = node.layeredBone->GetLocalTransform();
        }

        // Blend transformacji
        glm::quat rotBase = glm::quat_cast(transformBase);
        glm::quat rotLayer = glm::quat_cast(transformLayer);
        glm::quat blendedRot = glm::slerp(rotBase, rotLayer, blendFactor);
        glm::mat4 blendedTransform = glm::mat4_cast(blendedRot);
        blendedTransform[3] = (1.0f - blendFactor) * transformBase[3] + blendFactor * transformLayer[3];

        glm::mat4 parentTransform = (node.parentIndex >= 0) ? globalTransforms[node.parentIndex] : glm::mat4(1.0f);
        glm::mat4 globalTransform = parentTransform * blendedTransform;

        globalTransforms[i] = globalTransform;

        if (node.baseBone) {
            int id = m_CurrentAnimation->GetBoneIDMap().at(node.name).id;
            m_FinalBoneMatrices[id] = globalTransform * node.offset;
        }
    }
}

void Animator::BuildBlendedFlatBoneHierarchy(Animation *base, Animation *layered) {
    m_FlatBlendedNodes.clear();

    std::function<void(const AssimpNodeData *, const AssimpNodeData *, int)> recurse;
    recurse = [&](const AssimpNodeData *baseNode, const AssimpNodeData *layeredNode, int parentIndex) {
        FlatBoneNodeBlended flatNode;
        flatNode.name = baseNode->name;
        flatNode.parentIndex = parentIndex;
        flatNode.baseBindTransform = baseNode->transformation;
        flatNode.layeredBindTransform = layeredNode->transformation;
        flatNode.offset = glm::mat4(1.0f);

        auto &boneMapBase = base->GetBoneIDMap();
        auto &boneMapLayered = layered->GetBoneIDMap();

        if (boneMapBase.find(flatNode.name) != boneMapBase.end()) {
            flatNode.baseBone = base->FindBone(flatNode.name);
            flatNode.offset = boneMapBase.at(flatNode.name).offset;
        }

        if (boneMapLayered.find(flatNode.name) != boneMapLayered.end()) {
            flatNode.layeredBone = layered->FindBone(flatNode.name);
        }

        int thisIndex = m_FlatBlendedNodes.size();
        m_FlatBlendedNodes.push_back(flatNode);

        for (size_t i = 0; i < baseNode->children.size(); ++i) {
            if (i < layeredNode->children.size()) {
                recurse(&baseNode->children[i], &layeredNode->children[i], thisIndex);
            }
        }
    };

    recurse(&base->GetRootNode(), &layered->GetRootNode(), -1);
}

void Animator::SetBlendFactor(float blendFactor) {
    m_Blending = true;
    this->m_BlendFactor = blendFactor;
}

float Animator::GetBlendFactor() const { return m_BlendFactor; }

void Animator::SetBlending(bool blending) { m_Blending = blending; }

bool Animator::IsBlending() const { return m_Blending; }

void Animator::SetLayeredAnimation(const std::string &name) {

    auto anim = GetAnimation(name);

    if (anim) {
        m_CurrentLayeredAnimation = anim;
        return;
    }
    EngineDebug::GetInstance().PrintError("Animation " + name + " not found in animator.");
}

void Animator::DeselectLayeredAnimation() { m_CurrentLayeredAnimation = nullptr; }

int Animator::GetIndexOfLayeredAnimation() const {

    int index = -1;

    for (int i = 0; i < m_Animations.size(); i++) {
        if (m_Animations[i] == m_CurrentLayeredAnimation) {
            index = i;
        }
    }

    return index;
}

void Animator::AddAnimationTransition(AnimationTransition &transition) {

    auto it = std::find_if(m_AnimationTransitions.begin(), m_AnimationTransitions.end(),
                           [&](const AnimationTransition &t) { return t.name == transition.name; });
    if (it != m_AnimationTransitions.end()) {
        EngineDebug::GetInstance().PrintInfo("Can't add animation transition " + transition.name +
                                             " to animator - Animation transition already exists.");
        return;
    }
    m_AnimationTransitions.push_back(transition);
}

const std::vector<AnimationTransition> &Animator::GetAnimationTransitions() { return m_AnimationTransitions; }

void Animator::PlayTransition(const std::string &name) {

    auto it = std::find_if(m_AnimationTransitions.begin(), m_AnimationTransitions.end(),
                           [&](const AnimationTransition &t) { return t.name == name; });
    if (it == m_AnimationTransitions.end())
    {
        EngineDebug::GetInstance().PrintError("Can't play animation transition " + name +
            " - Animation transition not found.");
        return;
    }

    auto oldAnim = m_CurrentAnimation;
    float oldTime = m_CurrentTime;

    m_CurrentAnimation = it->animationBase;    
    m_CurrentLayeredAnimation = it->animationTarget;   
    m_Playing = true;
    m_Blending = true;
    m_TransitionTime = it->duration;
    m_TransitionDuration = it->duration;

    timeBase = (oldAnim == m_CurrentAnimation ? oldTime : 0.0f);
    timeLayer = 0.0f;

    m_FlatBlendedNodes.clear();
    BuildBlendedFlatBoneHierarchy(m_CurrentAnimation.get(), m_CurrentLayeredAnimation.get());
}

std::shared_ptr<Animation> Animator::GetCurrentAnimation() const { return m_CurrentAnimation; }

std::shared_ptr<Animation> Animator::GetLayeredAnimation() const { return m_CurrentLayeredAnimation; }

const std::vector<std::shared_ptr<Animation>> &Animator::GetAllAnimations() { return m_Animations; }

void Animator::RemoveAnimationTransition(AnimationTransition &transition) {

    auto it = std::remove_if(m_AnimationTransitions.begin(), m_AnimationTransitions.end(),
                             [&](const AnimationTransition &t) { return t.name == transition.name; });
    if (it != m_AnimationTransitions.end()) {
        m_AnimationTransitions.erase(it, m_AnimationTransitions.end());
    } else {
        EngineDebug::GetInstance().PrintError("Can't remove animation transition " + transition.name +
                                              " from animator - Animation transition not found.");
    }
}

float Animator::GetAnimationProgress() const {
    if (m_CurrentLayeredAnimation) {
        float durationInTicks = m_CurrentLayeredAnimation->GetDuration();
        if (durationInTicks > 0.0f) {
            return glm::clamp(timeLayer / durationInTicks, 0.0f, 1.0f);
        }
    }

    if (m_CurrentAnimation) {
        float durationInTicks = m_CurrentAnimation->GetDuration();
        if (durationInTicks > 0.0f) {
            return glm::clamp(m_CurrentTime / durationInTicks, 0.0f, 1.0f);
        }
    }
    return 0.0f;
}
