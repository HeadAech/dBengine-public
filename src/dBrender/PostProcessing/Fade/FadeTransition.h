#include <functional>
#ifndef FADE_TRANSITION_H
#define FADE_TRANSITION_H

enum class FadeState
{
	None,
	FadingOut,
	FadingIn
};

struct FadeTransition
{
	float fade = 0.0f;
	float fadeSpeed = 1.0f;

	FadeState state = FadeState::None;

	std::function<void()> onFadeOutComplete;

	FadeTransition() = default;
	FadeTransition(const FadeTransition& ft) = delete;
    FadeTransition &operator=(const FadeTransition &ft) = delete;

	static FadeTransition &GetInstance() {
        static FadeTransition instance;
        return instance;
    }


	void Start(std::function<void()> onFadeOutCompleteCallback)
	{
		if (state == FadeState::None)
		{
			state = FadeState::FadingOut;
			onFadeOutComplete = onFadeOutCompleteCallback;
		}
	}

	void Update(float deltaTime)
	{
        if (state == FadeState::FadingOut) {
            fade += fadeSpeed * deltaTime;
            if (fade >= 1.0f) {
                fade = 1.0f;
                state = FadeState::FadingIn;
                if (onFadeOutComplete) {
                    onFadeOutComplete();
                }
            }
        } else if (state == FadeState::FadingIn) {
            fade -= fadeSpeed * deltaTime;
            if (fade <= 0.0f) {
                fade = 0.0f;
                state = FadeState::None;
            }
        }
	}

	bool IsActive() const
	{
		return state != FadeState::None;
	}
		

};

#endif // !FADE_TRANSITION_H
