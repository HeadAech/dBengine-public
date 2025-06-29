#include "AnimationTransitionPanel.h"
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Helpers/Colors/Colors.h>

AnimationTransitionPanel::AnimationTransitionPanel() {

	SetName("Animation Transitions");

}

void AnimationTransitionPanel::Draw() {

	if (!p_Animator || !m_Visible)
        return;

    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_Appearing);
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 250), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin(GetName().c_str(), &m_Visible);

	const std::vector<AnimationTransition> &transitions = p_Animator->GetAnimationTransitions();

    if (ImGui::Button(ICON_FA_PLUS " Create Transition...")) {
        m_CreateModalVisible = true;
    }

	for (const auto &transition: transitions) {
        ImGui::PushItemWidth(-1);
        if (ImGui::TreeNode((ICON_FA_EXCHANGE " " + transition.name).c_str())) {
            ImGui::Text("Duration: %.2f", transition.duration);
            ImGui::Text("Base Animation: %s", transition.animationBase->GetName().c_str());
            ImGui::Text("Target Animation: %s", transition.animationTarget->GetName().c_str());
            if (ImGui::Button(ICON_FA_PLAY " Play")) {
                p_Animator->PlayTransition(transition.name);
            }
            ImGui::TreePop();
        }
        ImGui::PopItemWidth();
    }

    drawCreateTransitionModal();

	ImGui::End();

}

void AnimationTransitionPanel::Open() { m_Visible = true; }

void AnimationTransitionPanel::Close() { m_Visible = false; }

void AnimationTransitionPanel::drawCreateTransitionModal() {
    static char name[256];
    static float duration = 1.0f;
    static int baseAnimIndex = -1;
    static int targetAnimIndex = -1;

    if (m_CreateModalVisible) {
        ImGui::OpenPopup("Create Animation Transition");
        m_CreateModalVisible = false;
        duration = 1.0f;
        baseAnimIndex = -1;
        targetAnimIndex = -1;
        std::fill(std::begin(name), std::end(name), 0);
    }

    if (ImGui::BeginPopupModal("Create Animation Transition", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    
        
        const std::vector<std::string> animations = p_Animator->GetAllAnimationsNames();

        ImGui::Text("Name:");
        ImGui::InputText("##TransitionName", name, sizeof(name));

        ImGui::Text("Duration:");
        ImGui::DragFloat("##TransitionDuration", &duration, 0.01f, 0.0f, 2.0f);

        ImGui::Text("Base Animation:");
        if (ImGui::BeginCombo("##BaseAnimationCombo",
                              baseAnimIndex == -1 ? "Not Selected" : animations.at(baseAnimIndex).c_str())) {
            if (ImGui::Selectable("Not Selected", baseAnimIndex == -1)) {
                baseAnimIndex = -1;
            }
            for (int i = 0; i < animations.size(); i++) {
                bool isSelected = (baseAnimIndex == i);
                if (ImGui::Selectable(animations.at(i).c_str(), isSelected)) {
                    baseAnimIndex = i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Target Animation:");
        if (ImGui::BeginCombo("##TargetAnimationCombo",
                              targetAnimIndex == -1 ? "Not Selected" : animations.at(targetAnimIndex).c_str())) {
            if (ImGui::Selectable("Not Selected", targetAnimIndex == -1)) {
                targetAnimIndex = -1;
            }
            for (int i = 0; i < animations.size(); i++) {
                bool isSelected = (targetAnimIndex == i);
                if (ImGui::Selectable(animations.at(i).c_str(), isSelected)) {
                    targetAnimIndex = i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        float width = 100.0f;

        ImGui::PushItemWidth(-1);

        Panel::CenterWidget(width);

        ImGui::BeginGroup();
        ImGui::BeginDisabled(baseAnimIndex == -1 || targetAnimIndex == -1 || name == "");
        if (Panel::ColoredButton(ICON_FA_CHECK " Create", Colors::DarkGreen, ImVec2(width, 0))) {
            if (baseAnimIndex != -1 && targetAnimIndex != -1) {
                AnimationTransition transition;
                transition.name = name;
                transition.duration = duration;
                transition.animationBase = p_Animator->GetAnimation(animations.at(baseAnimIndex));
                transition.animationTarget = p_Animator->GetAnimation(animations.at(targetAnimIndex));
                p_Animator->AddAnimationTransition(transition);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        if (ImGui::Button(ICON_FA_BAN " Cancel", ImVec2(width, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndGroup();

        ImGui::PopItemWidth();
        
        ImGui::EndPopup();
    }

}


void AnimationTransitionPanel::SetAnimator(Animator *animator) { p_Animator = animator; }
