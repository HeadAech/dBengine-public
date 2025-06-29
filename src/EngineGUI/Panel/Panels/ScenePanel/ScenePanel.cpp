#include "ScenePanel.h"
#include <imgui_internal.h>
#include <EngineGUI/EngineGUI.h>
#include <dBengine/EngineDebug/EngineDebug.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Signal/Signals.h>
#include <Helpers/Colors/Colors.h>
#include <Singletons/Ref/Ref.h>
ScenePanel::ScenePanel() { SetName("Scene"); }

void ScenePanel::Draw() {
    EngineGUI &gui = EngineGUI::GetInstance();

    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowDockID(ImGui::GetID("LeftDockSpace"), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scene Hierarchy", NULL)) {
        if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered()) {
            ImGui::OpenPopup("SceneTreeContextMenu");
        }

        // right click menu
        drawContextMenu();

        drawSceneTree(gui.GetScene());

        drawConfirmationDelete();
        drawRenameModal();
        drawWarningLoadSceneModal();
        
        
    }
    m_CreatePanel.Draw();
    m_AppendPanel.Draw();

    ImGui::End();
}

void ScenePanel::drawContextMenu() {
    if (ImGui::BeginPopup("SceneTreeContextMenu")) {

        if (ImGui::MenuItem(ICON_FA_CAMERA " Return To Scene Camera")) {
            Signals::Engine_ReturnToSceneCamera.emit(false);
        }
        ImGui::EndPopup();
    }
}

void ScenePanel::drawConfirmationDelete() { 
    if (m_DeleteConfirmationVisible) {
        m_DeleteConfirmationVisible = false;
        ImGui::OpenPopup("Confirm Deletion");
    }

    if (ImGui::BeginPopupModal("Confirm Deletion", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::string deleteOutput = "Are you sure you want to delete this Game Object: " + p_GameObjectToDelete->name + "?";
        ImGui::Text(deleteOutput.data());
        ImGui::Spacing();

        float width = 140.0f;
        Panel::CenterWidget(width);
        ImGui::BeginGroup();

        if (ImGui::Button("Cancel", ImVec2(width, 0))) {
            p_GameObjectToDelete = nullptr;
            ImGui::CloseCurrentPopup();
        }

        if (Panel::ColoredButton(ICON_FA_TRASH " Yes, Delete", Colors::DarkRed, ImVec2(width, 0))) {
            EngineGUI::GetInstance().GetScene()->DeleteGameObject(p_GameObjectToDelete->GetUUID());
            ImGui::CloseCurrentPopup();
            p_GameObjectToDelete = nullptr;
            EngineGUI::GetInstance().selectedGameObject = nullptr;
        }

        ImGui::EndGroup();

        ImGui::EndPopup();
    }
}

void ScenePanel::drawSceneTree(Scene* scene) {
    if (!scene)
        return;

    drawGameObjectSceneTree(scene->sceneRootObject.get());
}



void ScenePanel::drawGameObjectSceneTree(GameObject *gameObject) {
    if (gameObject == nullptr)
        return;
    ImGui::PushID(gameObject->GetUUID().c_str());
    EngineGUI &gui = EngineGUI::GetInstance();

    bool gameObjectEnabled = gameObject->m_enabled;

    if (ImGui::Checkbox(("##" + gameObject->GetUUID() + "chckbx").c_str(), &gameObjectEnabled)) {
        if (gameObjectEnabled) {
            gameObject->Enable();
        } else {
            gameObject->Disable();
        }
    }
    ImGui::SameLine();
    bool hasChildren = gameObject->children.size() > 0;


    ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!hasChildren || (gameObject->parent && gameObject->pathToScene != "")) {
        treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    if (gui.selectedGameObject == gameObject) {
        treeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    bool isSelectedOrAncestor = false;
    if (gui.selectedGameObject == gameObject) {
        isSelectedOrAncestor = true;
    } else if (gui.selectedGameObject != nullptr) {
        GameObject *current = gui.selectedGameObject;
        while (current != nullptr) {
            if (current->parent == gameObject) {
                isSelectedOrAncestor = true;
                break;
            }
            current = current->parent;
        }
    }

    if (isSelectedOrAncestor && hasChildren &&
        !(gameObject->parent && gameObject->pathToScene != "")) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    }

    std::string displayName = (gameObject->pathToScene == "") ? gameObject->name : std::string(ICON_FA_CUBES) + " " + gameObject->name;

    if (auto lua = gameObject->GetComponent<LuaComponent>())
    {
        displayName = ICON_FA_FILE_CODE_O " " + displayName;
    }

    bool imguiNodeTreeEx = ImGui::TreeNodeEx(displayName.c_str(), treeFlags);

    if (ImGui::IsItemClicked() || (ImGui::IsItemHovered && ImGui::IsItemClicked(1))) {
        gui.selectedGameObject = gameObject;
        gui.changedSelectedGameObject = true;
    }

    //right click
    if (ImGui::BeginPopupContextItem(("context_menu_" + gameObject->GetUUID()).c_str())) {

        if (gameObject->parent)
        {

            bool disabled = gameObject->parent->children.front()->GetUUID() == gameObject->GetUUID();
            ImGui::BeginDisabled(disabled);
            if (ImGui::MenuItem(ICON_FA_ARROW_UP " Move Up"))
            {
                // move up in hierrachy logic
                size_t index = 0;

                for (int i = 0; i < gameObject->parent->children.size(); i++) {
                    if (gameObject->parent->children[i]->GetUUID() == gameObject->GetUUID()) {
                        index = i;
                        break;
                    }
                }

                Util::MoveObject(gameObject->parent->children, index, true);
                Signals::Render_ResetFlatObjects.emit();
            }
            ImGui::EndDisabled();

            bool disabledDown = gameObject->parent->children.back()->GetUUID() == gameObject->GetUUID();

            ImGui::BeginDisabled(disabledDown);
            if (ImGui::MenuItem(ICON_FA_ARROW_DOWN " Move Down"))
            {
                // move down in hierrachy logic
                size_t index = 0;

                for (int i = 0; i < gameObject->parent->children.size(); i++)
                {
                    if (gameObject->parent->children[i]->GetUUID() == gameObject->GetUUID())
                    {
                        index = i;
                        break;
                    }
                }

                Util::MoveObject(gameObject->parent->children, index, false);
                Signals::Render_ResetFlatObjects.emit();

            }
            ImGui::EndDisabled();
        }
        

        ImGui::Separator();

        if ((gameObject->pathToScene == "" || !gameObject->parent ) && ImGui::BeginMenu(ICON_FA_PLUS_CIRCLE " Create Child...")) {

            if (ImGui::MenuItem("Empty")) {
                Signals::InstantiateEmptyGameObject.emit(gameObject);
            }

            ImGui::SeparatorText("Shapes");

            if (ImGui::MenuItem("Cube")) {
                Signals::InstantiateGameObject.emit("Cube", "res/models/cube/cube.obj", {0, 0, 0}, gameObject);
            }

            ImGui::SeparatorText("Other");

            if (ImGui::MenuItem("New...")) {
                m_CreatePanel.Open();
            }

            if (ImGui::MenuItem("Append Scene...")) {
                EngineGUI &gui = EngineGUI::GetInstance();
                m_AppendPanel.Open(gui.selectedGameObject);
            }

            ImGui::EndMenu();
        } else if (gameObject->pathToScene != "" && gameObject->parent) {
            if (ImGui::MenuItem(ICON_FA_FILE " Load Scene")) {
                m_WarningLoadScenePopup = true;
            } 
        
        
        }
            
        if (ImGui::MenuItem(ICON_FA_TRASH " Delete...")) {
            p_GameObjectToDelete = gameObject;
            m_DeleteConfirmationVisible = true;
        }
        if (ImGui::MenuItem(ICON_FA_PENCIL " Rename...")) {
            // Open rename input
            p_GameObjectToEdit = gameObject;
            m_RenameModalVisible = true;
        }
        if (ImGui::MenuItem("Duplicate")) {
            // Duplicate logic
        }
        if (gameObject->GetComponent<Camera>()) {
            if (ImGui::MenuItem(ICON_FA_CAMERA " Preview")) {
                gameObject->GetComponent<Camera>()->isUsed = true;
                Signals::Engine_UseCamera.emit(gameObject->uuid,false);
            }
        }
        ImGui::EndPopup();
    }
    // when no children, draw a selectable
    if (imguiNodeTreeEx && hasChildren && !(gameObject->parent && gameObject->pathToScene != "")) {
        for (const auto &child: gameObject->children)
            drawGameObjectSceneTree(child.get());
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void ScenePanel::drawRenameModal() { 

    if (m_RenameModalVisible) {
        m_RenameModalVisible = false;
        std::strncpy(m_NameBuffer, p_GameObjectToEdit->name.c_str(), sizeof(m_NameBuffer));
        ImGui::OpenPopup("Change GameObject's name");
    }

    if (ImGui::BeginPopupModal("Change GameObject's name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        if (m_NameConflictPopupVisible) {
            drawNameConflictPopup();
        } else {

            ImGui::InputText(("##gameObjectNameTextInput" + p_GameObjectToEdit->GetUUID()).c_str(), m_NameBuffer,
                             sizeof(m_NameBuffer));

            float width = 100.0f;

            Panel::CenterWidget(width);

            ImGui::BeginGroup();

            if (ImGui::Button(ICON_FA_FLOPPY_O " Save", ImVec2(width, 0))) {
                std::string_view oldName = m_NameBuffer;
                GameObject *GO = p_GameObjectToEdit->parent ? p_GameObjectToEdit->parent : p_GameObjectToEdit;

                if (dBrender::GetInstance().m_activeScene->CheckGameObjectNameSiblings(m_NameBuffer, GO) != oldName) {
                    m_NameConflictPopupVisible = true;
                } else {
                    p_GameObjectToEdit->name = m_NameBuffer;
                    p_GameObjectToEdit = nullptr;
                    p_GameObjectToDelete = nullptr;
                    ImGui::CloseCurrentPopup();
                }
            }

            if (Panel::ColoredButton("Cancel", Colors::DarkRed, ImVec2(width, 0))) {
                p_GameObjectToDelete = nullptr;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndGroup();
        }
        ImGui::EndPopup();
    }
}

void ScenePanel::drawNameConflictPopup() { 

    ImGui::TextColored(Colors::DarkRed, "GameObject with this name already exists.");

    if (ImGui::Button("Okay")) {
        m_NameConflictPopupVisible = false;
    }


}

void ScenePanel::drawWarningLoadSceneModal() {
    if (m_WarningLoadScenePopup) {
        m_WarningLoadScenePopup = false;
        ImGui::OpenPopup("Load GameObject Scene");
    }
    if (ImGui::BeginPopupModal("Load GameObject Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::TextColored(Colors::DarkRed, ("This one will load a scene, and oversave current one at its path at: scenes/" + Ref::CurrentScene->name).c_str());
        float width = 100.0f;

        Panel::CenterWidget(width);

        ImGui::BeginGroup();

        if (Panel::ColoredButton("Okay", Colors::LightGreen, ImVec2(width, 0))) {
            
            Signals::SceneToFile.emit("scenes/" + Ref::CurrentScene->name + ".yaml");
            Signals::FileToScene.emit(EngineGUI::GetInstance().selectedGameObject->pathToScene);
             
            ImGui::CloseCurrentPopup();
        }

        if (Panel::ColoredButton("Cancel", Colors::DarkRed, ImVec2(width, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndGroup();
        
        ImGui::EndPopup();
    }
}