#include "AnimationLibrary.h"
#include <Signal/Signals.h>

AnimationLibrary &AnimationLibrary::GetInstance() {
    static AnimationLibrary instance;
    return instance;
}

void AnimationLibrary::AddAnimation(const std::string &name, const std::string &animationPath) {
    if (GetAnimation(name)) {
        EngineDebug::GetInstance().PrintWarning("Animation " + name + " already exists in the library.");
        return;
    }
	m_Animations[name] = std::make_shared<Animation>(name, animationPath);
}

void AnimationLibrary::DeleteAnimation(std::shared_ptr<Animation> &animation) {
    auto it = m_Animations.find(animation->GetName());
    if (it != m_Animations.end()) {
        Signals::AnimationLibrary_AnimationDeleted.emit(animation);
        m_Animations.erase(it);
    }
}

std::shared_ptr<Animation> AnimationLibrary::GetAnimation(const std::string &name) { 
	if (m_Animations.find(name) != m_Animations.end()) {
        return m_Animations[name];
	}

	return nullptr;
}

const std::unordered_map<std::string, std::shared_ptr<Animation>>& AnimationLibrary::GetAllAnimations() {
    return m_Animations;
}
