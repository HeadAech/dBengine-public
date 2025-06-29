#include "AddComponentPanel.h"
#include <EngineGUI/EngineGUI.h>
#include <Helpers/Colors/Colors.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Components/Control/Button/Button.h>
#include <Components/Control/Text/Text.h>
#include <Serializers/SceneSerializer.h>
#include <Components/Control/Sprite/Sprite.h>
#include <Singletons/Ref/Ref.h>

AddComponentPanel::AddComponentPanel() { SetName("Add Component"); }

void AddComponentPanel::Draw() {
    EngineGUI &gui = EngineGUI::GetInstance();

    if (m_Visible) {
        m_Visible = false;
        ImGui::OpenPopup("Add Component");
    }


    static std::vector<std::string> availableComponents = { 
        "Camera",          "Collision Shape",    "Lua Script",   
        "Mesh Instance",   "Directional Light",  "Point Light",     
        "Spot Light",      "Particle System",    "Animator",        
        "Physics Body",    "Tag",                "UI::Button",      
        "UI::Text",        "UI::Sprite",         "AISystem",      
        "AIAgent",         "NavigationMesh",     "NavigationTarget"
    };
    static int selectedComponentToAdd = 0;

    if (gui.changedSelectedGameObject) {
        selectedComponentToAdd = 0;
    }

    if (ImGui::BeginPopupModal("Add Component", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {


        ImGui::Text("Select a component:");
        if (ImGui::BeginCombo("##ComboComponentAdd", availableComponents.at(selectedComponentToAdd).c_str())) {

            for (int i = 0; i < availableComponents.size(); i++) {
                bool isSelected = (selectedComponentToAdd == i);
                if (ImGui::Selectable(availableComponents[i].c_str(), isSelected)) {
                    selectedComponentToAdd = i;
                }
            }

            ImGui::EndCombo();
        }

        float width = 100.0f;

        Panel::CenterWidget(width);

        ImGui::BeginGroup();

        if (Panel::ColoredButton(ICON_FA_PLUS " Add", Colors::DarkGreen, ImVec2(width, 0))) {
            if (availableComponents.at(selectedComponentToAdd) == "Lua Script") {
                gui.selectedGameObject->AddComponent<LuaComponent>();
            }
            else if (availableComponents.at(selectedComponentToAdd) == "Directional Light")
            {
                gui.selectedGameObject->AddComponent<DirectionalLight>();
            }
            else if (availableComponents.at(selectedComponentToAdd) == "Point Light") {
                gui.selectedGameObject->AddComponent<PointLight>();
            } else if (availableComponents.at(selectedComponentToAdd) == "Spot Light") {
                gui.selectedGameObject->AddComponent<SpotLight>();
            }
            else if (availableComponents.at(selectedComponentToAdd) == "Particle System")
            {
                auto ps = gui.selectedGameObject->AddComponent<ParticleSystem>();
                ps->SetShader(dBrender::GetInstance().GetParticlesShader());
            }
            else if (availableComponents.at(selectedComponentToAdd) == "Camera")
            {
                gui.selectedGameObject->AddComponent<Camera>(Ref::AspectRatio);
            }
            else if (availableComponents.at(selectedComponentToAdd) == "Mesh Instance")
            {
                gui.selectedGameObject->AddComponent<MeshInstance>();
            } 
            else if (availableComponents.at(selectedComponentToAdd) == "Physics Body") {
                auto physicsBody = gui.selectedGameObject->AddComponent<PhysicsBody>();
                physicsBody->useGravity = false;
            } 
            else if (availableComponents.at(selectedComponentToAdd) == "Tag") {
                gui.selectedGameObject->AddComponent<Tag>("New tag");
            } 
            else if (availableComponents.at(selectedComponentToAdd) == "Text Renderer") {
                //gui.selectedGameObject->AddComponent<TextRenderer>();
            } 
            else if (availableComponents.at(selectedComponentToAdd) == "Timer") {
                //gui.selectedGameObject->AddComponent<Timer>();
            } 
            else if (availableComponents.at(selectedComponentToAdd) == "Audio Listener") {
                //gui.selectedGameObject->AddComponent<AudioListener>();
            } 
            else if (availableComponents.at(selectedComponentToAdd) == "Audio Source") {
                gui.selectedGameObject->AddComponent<AudioSource>("",FMOD_STUDIO_LOAD_BANK_NORMAL);
            }
            else if (availableComponents.at(selectedComponentToAdd) == "UI::Button")
            {
                auto btn = gui.selectedGameObject->AddComponent<UI::Button>();
                btn->Position = { 0, 0 };
                btn->Size = { 100, 50 };
            } 
            else if (availableComponents.at(selectedComponentToAdd) == "UI::Text") {
                auto text = gui.selectedGameObject->AddComponent<UI::Text>();
                text->Position = { 0, 0 };
                text->Size = { 100, 50 }; 
            }
            else if (availableComponents.at(selectedComponentToAdd) == "Animator")
            {
                auto animator = gui.selectedGameObject->AddComponent<Animator>();
            } else if (availableComponents.at(selectedComponentToAdd) == "Collision Shape") {
                auto collisionShape = gui.selectedGameObject->AddComponent<CollisionShape>();
            } else if (availableComponents.at(selectedComponentToAdd) == "UI::Sprite") {
                auto sprite = gui.selectedGameObject->AddComponent<UI::Sprite>();
                sprite->Position = {0, 0};
                sprite->Size = {100, 100};
            } else if (availableComponents.at(selectedComponentToAdd) == "AISystem") {
                gui.selectedGameObject->AddComponent<AISystem>();
            } else if (availableComponents.at(selectedComponentToAdd) == "AIAgent") {
                gui.selectedGameObject->AddComponent<AIAgent>();
            } else if (availableComponents.at(selectedComponentToAdd) == "NavigationMesh") {
                gui.selectedGameObject->AddComponent<NavigationMesh>();
            } else if (availableComponents.at(selectedComponentToAdd) == "NavigationTarget") {
                gui.selectedGameObject->AddComponent<NavigationTarget>();
            }

            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel", ImVec2(width, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndGroup();

        ImGui::EndPopup();
    }
}

void AddComponentPanel::Open() { 
    m_Visible = true; 
    ImGui::OpenPopup("Add Component...");
}

void AddComponentPanel::Close() { m_Visible = false; }
