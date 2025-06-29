#ifndef ANIMATION_TRANSITION_PANEL_H
#define ANIMATION_TRANSITION_PANEL_H

#include "../../Panel.h"
#include <Components/Animator/Animator.h>

class AnimationTransitionPanel : public Panel {

	public:

		AnimationTransitionPanel();
        void Draw() override;
        
		void Open();
        void Close();

		void SetAnimator(Animator *animator);
	private:
		
		Animator *p_Animator = nullptr;
        
		bool m_Visible = false;

		bool m_CreateModalVisible = false;

		void drawCreateTransitionModal();

};

#endif // !ANIMATION_TRANSITION_PANEL_H
