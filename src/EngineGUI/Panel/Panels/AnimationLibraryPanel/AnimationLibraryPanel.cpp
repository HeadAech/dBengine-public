#include "AnimationLibraryPanel.h"
#include <Components/Animator/AnimationLibrary/AnimationLibrary.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <ImGuiFileDialog.h>
#include <Helpers/Colors/Colors.h>

AnimationLibraryPanel::AnimationLibraryPanel() { SetName("Animation Library"); }

void AnimationLibraryPanel::Draw() {
    if (!m_Visible)
        return;

    AnimationLibrary &animLib = AnimationLibrary::GetInstance();

    std::unordered_map<std::string, std::shared_ptr<Animation>> animations = animLib.GetAllAnimations();

    ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_Appearing);
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 400), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin(GetName().c_str(), &m_Visible);

    if (ImGui::Button(ICON_FA_PLUS " Add Animation")) {
        // open file dialog to select animation
        IGFD::FileDialogConfig config;
        config.path = "./res/models";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseAnimation", "Choose Animation...", "Animation files (*.fbx *.gltf *.glb *.dae){.fbx,.gltf,.glb,.dae}", config);
    }

    Panel::ColoredText("Drag and drop the animation on the animator's combo box to add it to the animator.", Colors::Gray);

    if (ImGuiFileDialog::Instance()->Display("ChooseAnimation", NULL, ImVec2(400, 300))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string selectedAnimationPath = Util::getRelativePath(ImGuiFileDialog::Instance()->GetFilePathName()).string();
            std::filesystem::path path = std::filesystem::path(selectedAnimationPath);
            animLib.AddAnimation(path.stem().string(), path.generic_string());
        }
        ImGuiFileDialog::Instance()->Close();
    }
    
    if (ImGui::BeginTable("Loaded Animations", 2, ImGuiTableFlags_BordersInnerH)) {
        // headers
        ImGui::TableSetupColumn("Animation Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        for (auto it = animations.begin(); it != animations.end(); it++) {
            std::string name = it->first;
            std::shared_ptr<Animation> pAnim = it->second;

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            
           
            ImGui::Selectable((ICON_FA_FILM " " + name).c_str());
            // Begin drag-drop source for any entry
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("Animation_DND", name.c_str(), name.size() + 1);
                ImGui::Text((ICON_FA_FILM " " + name).c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(name.c_str());
            if (ImGui::Button(ICON_FA_TRASH " Delete")) {
                animLib.DeleteAnimation(pAnim);
            }
            ImGui::PopID();

        }

        ImGui::EndTable();
    }
    

    ImGui::End();
}

void AnimationLibraryPanel::Open() { m_Visible = true; }

void AnimationLibraryPanel::Close() { m_Visible = false; }
