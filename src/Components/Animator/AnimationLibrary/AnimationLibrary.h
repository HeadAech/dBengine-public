#include <vector>
#include <Components/Animator/Animation/Animation.h>
#include <memory>
#ifndef ANIMATION_LIBRARY_H
#define ANIMATION_LIBRARY_H

class AnimationLibrary {
	public:
		AnimationLibrary() = default;
        ~AnimationLibrary() = default;

		AnimationLibrary(const AnimationLibrary &) = delete;
        AnimationLibrary &operator=(const AnimationLibrary &) = delete;

        static AnimationLibrary &GetInstance();

		void AddAnimation(const std::string &name, const std::string &animationPath);
        void DeleteAnimation(std::shared_ptr<Animation> &animation);

		std::shared_ptr<Animation> GetAnimation(const std::string &name);

		const std::unordered_map<std::string, std::shared_ptr<Animation>>& GetAllAnimations();
	private:

		std::unordered_map<std::string, std::shared_ptr<Animation>> m_Animations;

};

#endif // !ANIMATION_LIBRARY_H
