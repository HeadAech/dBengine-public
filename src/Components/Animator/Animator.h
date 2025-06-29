#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <Component/Component.h>
#include "Animation/Animation.h"
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/glm.hpp>


struct AnimationTransition {
    std::string name;
    float duration;

    std::shared_ptr<Animation> animationBase;
    std::shared_ptr<Animation> animationTarget;
};



class Animator : public Component {
    public:

        Animator();
        ~Animator();

        void Update(float deltaTime) override;

        void PlayAnimation(const std::string& animName);

        void CalculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform,
                                    std::map<std::string, BoneInfo> boneInfoMap);

        void EvaluateFlatBoneHierarchy(const std::vector<FlatBoneNode> &flatNodes);


        std::vector<glm::mat4> GetFinalBoneMatrices();


        bool IsPlaying();

        void Pause();
        void Play();
        void Reset();

        void AddAnimation(std::shared_ptr<Animation> &animation);
        void RemoveAnimation(std::shared_ptr<Animation> animation);
        std::shared_ptr<Animation> GetAnimation(const std::string &animName);

        Animation *GetCurrentAnimation();

        float GetCurrentPlaybackTime();

        void SetCurrentPlaybackTime(float time);

        std::vector<std::string> GetAllAnimationsNames() const;
        int GetIndexOfCurrentAnimation() const;

        void SetCurrentAnimation(int index);
        void SetCurrentAnimation(std::string name);
        void DeselectAnimation();

        float GetTimeScale() const;
        void SetTimeScale(float timeScale);


        void BlendTwoAnimations(Animation *pBaseAnimation, Animation *pLayeredAnimation, float blendFactor,
                                float deltaTime);

        void CalculateBlendedBoneTransform(Animation *pAnimationBase, const AssimpNodeData *node,
                                           Animation *pAnimationLayer, const AssimpNodeData *nodeLayered,
                                           const float currentTimeBase, const float currentTimeLayered,
                                           const glm::mat4 &parentTransform, const float blendFactor);

        void EvaluateBlendedFlatBoneHierarchy(const std::vector<FlatBoneNodeBlended> &flatNodes, float currentTimeBase,
                                              float currentTimeLayered, float blendFactor);

        void BuildBlendedFlatBoneHierarchy(Animation *base, Animation *layered);

        void SetBlendFactor(float blendFactor);
        float GetBlendFactor() const;
        void SetBlending(bool blending);
        bool IsBlending() const;

        void SetLayeredAnimation(const std::string &name);
        void DeselectLayeredAnimation();

        int GetIndexOfLayeredAnimation() const;

        void AddAnimationTransition(AnimationTransition &transition);
        void RemoveAnimationTransition(AnimationTransition &transition);

        const std::vector<AnimationTransition> &GetAnimationTransitions();

        void PlayTransition(const std::string &name);

        std::shared_ptr<Animation> GetCurrentAnimation() const;
        std::shared_ptr<Animation> GetLayeredAnimation() const;

        float GetAnimationProgress() const;
        
        const std::vector<std::shared_ptr<Animation>> &GetAllAnimations();

    private:

        std::vector<glm::mat4> m_FinalBoneMatrices;
        
        std::vector<std::shared_ptr<Animation>> m_Animations;
        std::shared_ptr<Animation> m_CurrentAnimation = nullptr;


        float m_CurrentTime = 0.0f;
        float m_DeltaTime;

        float m_TimeScale = 1.0f;

        bool m_Playing = false;

        bool m_Blending = false;
        float m_BlendFactor = 0.0f;

        float m_TransitionTime = 0.0f;
        float m_TransitionDuration = 0.0f;

        std::shared_ptr<Animation> m_CurrentLayeredAnimation = nullptr;

        std::vector<FlatBoneNodeBlended> m_FlatBlendedNodes;

        std::vector<AnimationTransition> m_AnimationTransitions;

        float timeBase = 0.0f;
        float timeLayer = 0.0f;
};

#endif // !ANIMATOR_H
