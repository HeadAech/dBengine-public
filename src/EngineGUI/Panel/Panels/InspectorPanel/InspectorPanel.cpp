#include "InspectorPanel.h"
#include <imgui_internal.h>
#include <EngineGUI/EngineGUI.h>
#include <Helpers/Util.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Components/LuaComponent/LuaComponent.h>
#include <Components/Lights/SpotLight/SpotLight.h>
#include <Components/Lights/PointLight/PointLight.h>
#include <Components/TextRenderer/TextRenderer.h>
#include <Components/MeshInstance/MeshInstance.h>
#include <Components/Animator/Animator.h>
#include <Helpers/Colors/Colors.h>
#include <Components/Animator/AnimationLibrary/AnimationLibrary.h>
#include <ResourceManager/ResourceManager.h>
#include <Components/PhysicsBody/PhysicsBody.h>
#include <Components/CollisionShape/CollisionShape.h>
#include <ImGuiFileDialog.h>
#include "Component/Component.h"
#include <Components/Control/Button/Button.h>
#include <Components/ThirdPersonCamera/ThirdPersonCamera.h>
#include <Components/Control/Text/Text.h>
#include <Components/Control/Sprite/Sprite.h>
#include <Components/AISystem/AISystem.h>

InspectorPanel::InspectorPanel() { SetName("Inspector"); }

void InspectorPanel::Draw() {
    EngineGUI &gui = EngineGUI::GetInstance();

    m_AddComponentPanel.Draw();


    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 200), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Inspector")) {

        if (gui.selectedGameObject) {

            static bool showChangeNameInput = false;

         
            static char nameBuffer[256];


            if (gui.changedSelectedGameObject) {
                gui.changedSelectedGameObject = false;
                showChangeNameInput = false;
            }

            ImGui::Text("Selected:");

            if (!showChangeNameInput) {
                ImGui::SameLine();
                ImGui::Text(gui.selectedGameObject->name.c_str());
            }

            ImGui::Spacing();

            ImGui::SeparatorText("Components");
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_FA_PLUS " Add...")) {
                m_AddComponentPanel.Open();
            }

            ImGui::Spacing();

            if (Panel::CollapsingHeader( " " ICON_FA_ARROWS "  Transform")) {
                ImGui::Indent();
                // translation
                glm::vec3 positionVec = gui.selectedGameObject->transform.GetLocalPosition();
                if (Panel::DragVec3Widget("Translation", positionVec, 0.0f, 100.0f)) {
                    gui.selectedGameObject->transform.SetLocalPosition(positionVec); 
                }

                ImGui::Spacing();

                // rotation
                glm::vec3 rotationVec = gui.selectedGameObject->transform.GetEulerRotation();
                glm::vec3 oldRotationVec = rotationVec;
                if (Panel::DragVec3Widget("Rotation", rotationVec, 0.0f)) {
                    glm::vec3 delta = rotationVec - oldRotationVec;
                    gui.selectedGameObject->transform.Rotate(delta);
                    Camera *cam = gui.selectedGameObject->GetComponent<Camera>();
                    if (cam) {
                        cam->Yaw += delta.x;
                        cam->Pitch += delta.y; 
                        cam->UpdateCameraVectors();
                    }
                }

                ImGui::Spacing();

                // scale
                glm::vec3 sclVec = gui.selectedGameObject->transform.scale;
                if (Panel::DragVec3Widget("Scale", sclVec, 1.0f)) {
                    gui.selectedGameObject->transform.SetScale(sclVec);
                }
                ImGui::Spacing();
                ImGui::Spacing();

                ImGui::Unindent();
            }

            if (gui.selectedGameObject->components.size() > 0) {
                for (const auto &child: gui.selectedGameObject->components) {
                    drawComponentSceneTree(child.get());
                }
            }

            ImGui::Spacing();
            ImGui::Separator();

        } else {
            ImGui::Text("No selected GameObject");
        }
    }
    ImGui::End();

}


void InspectorPanel::drawComponentSceneTree(Component* component) {
    if (!component)
        return;

    EngineGUI &gui = EngineGUI::GetInstance();

    float availWidth = ImGui::GetContentRegionAvail().x;

    bool componentEnabled = component->enabled;
    if (ImGui::Checkbox(("##EnabledComponent" + component->name + component->gameObject->GetUUID()).c_str(),
                        &componentEnabled)) {
        if (componentEnabled) {
            component->Enable();
        } else {
            component->Disable();
        }
    }
    ImGui::SameLine();

    float lineHeight = ImGui::GetTextLineHeightWithSpacing();
    ImVec2 buttonSize = ImVec2(lineHeight, lineHeight);

    bool toDelete = false;

    if (Panel::CollapsingHeader(( " " + component->icon + "  " + component->name).c_str(), ICON_FA_TRASH, [&toDelete]()
        {
            toDelete = true;
        })) {
        ImGui::Indent();
        ImGui::BeginGroup();
        if (LuaComponent *luaComponent = dynamic_cast<LuaComponent *>(component)) {

            drawLuaComponentPanel(luaComponent);
        } 
        else if (TextRenderer *textRenderer = dynamic_cast<TextRenderer *>(component)) 
        {
            drawTextRendererComponentPanel(textRenderer);
        } 
        else if (Light *light = dynamic_cast<Light *>(component)) 
        {
            // general shared light properties
            drawLightComponentPanel(light);

            // local lights specific properties
            if (LocalLight *localLight = dynamic_cast<LocalLight *>(component)) {
                drawLocalLightComponentPanel(localLight);
            }

            // spotlight specific properties
            if (SpotLight *spotLight = dynamic_cast<SpotLight *>(component)) {
                drawSpotLightComponentPanel(spotLight);
            }
        } 
        else if (WorldEnvironment *worldEnv = dynamic_cast<WorldEnvironment *>(component)) 
        {
            drawWorldEnvComponentPanel(worldEnv);
        }
        else if (MeshInstance *meshInstance = dynamic_cast<MeshInstance *>(component)) 
        {
            drawMeshInstanceComponentPanel(meshInstance);
        } 
        else if (PhysicsBody *physicsBody = dynamic_cast<PhysicsBody *>(component)) {
            float currentMass = physicsBody->mass;
            ImGui::Text("Mass:");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            if (ImGui::DragFloat("##PhysicsBodyMass", &currentMass, 0.1f, 0.1f, 1000.0f)) {
                physicsBody->SetMass(currentMass);
            }
            ImGui::PopItemWidth();

            bool isStaticCurrent = physicsBody->isStatic;
            if (ImGui::Checkbox("Is Static", &isStaticCurrent)) {
                physicsBody->SetStatic(isStaticCurrent);
            }

            ImGui::Checkbox("Use Gravity", &physicsBody->useGravity);

            ImGui::Text("Bounciness:");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##PhysicsBodyRestitution", &physicsBody->restitution, 0.01f, 0.0f, 1.0f);
            ImGui::PopItemWidth();

            ImGui::Text("Friction:");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##PhysicsBodyLinearDamping", &physicsBody->linearDamping, 0.01f, 0.0f, 1.0f);
            ImGui::PopItemWidth();
        } else if (CollisionShape *collisionShape = dynamic_cast<CollisionShape *>(component)) {
            static std::vector<std::string> shapeTypes = {"BOX", "CAPSULE"};
            int currentShapeType = static_cast<int>(collisionShape->GetShapeType());

            ImGui::Text("Shape Type:");
            ImGui::PushItemWidth(-1);
            if (ImGui::BeginCombo("##CollisionShapeType", shapeTypes.at(currentShapeType).c_str())) {
                for (int i = 0; i < shapeTypes.size(); i++) {
                    bool isSelected = (currentShapeType == i);
                    if (ImGui::Selectable(shapeTypes[i].c_str(), isSelected)) {
                        collisionShape->SetShapeType(static_cast<ShapeType>(i));
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();

            ImGui::Spacing();

            if (collisionShape->GetShapeType() == ShapeType::BOX) {
                glm::vec3 boxSize = collisionShape->GetBoxSize();
                ImGui::Text("Box Size:");
                ImGui::PushItemWidth(-1);
                if (ImGui::DragFloat3("##CollisionBoxSize", (float *) &boxSize, 0.1f, 0.1f, 100.0f)) {
                    collisionShape->SetBoxSize(boxSize);
                }
                ImGui::PopItemWidth();
            } else if (collisionShape->GetShapeType() == ShapeType::CAPSULE) {
                float radius = collisionShape->GetCapsuleRadius();
                ImGui::Text("Radius:");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                if (ImGui::DragFloat("##CollisionCapsuleRadius", &radius, 0.01f, 0.1f, 10.0f)) {
                    collisionShape->SetCapsuleParams(radius, collisionShape->GetCapsuleHeight());
                }
                ImGui::PopItemWidth();

                float height = collisionShape->GetCapsuleHeight();
                ImGui::Text("Height:");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                if (ImGui::DragFloat("##CollisionCapsuleHeight", &height, 0.01f, 0.1f, 20.0f)) {
                    collisionShape->SetCapsuleParams(collisionShape->GetCapsuleRadius(), height);
                }
                ImGui::PopItemWidth();
            }

            ImGui::Spacing();
            ImGui::Separator();

            bool isVisible = collisionShape->IsVisible();
            if (ImGui::Checkbox("Visible", &isVisible)) {
                collisionShape->SetVisible(isVisible);
            }

            bool isArea = collisionShape->GetIsCollisionArea();
            if (ImGui::Checkbox("Is Area Collision", &isArea)){
                collisionShape->SetIsCollisionArea(isArea);
            }




            ImGuiInputTextFlags textFlags = ImGuiInputTextFlags_EnterReturnsTrue;
            static char textBuff[256];
            std::strncpy(textBuff, collisionShape->SignalMessage_OnAreaEntered.c_str(), sizeof(textBuff));
            ImGui::Text("OnAreaEntered Message:");
            ImGui::SameLine();
            if (ImGui::InputText("##OnAreaEnteredInput", textBuff, sizeof(textBuff), textFlags)) {
                collisionShape->SignalMessage_OnAreaEntered = textBuff;
                std::cout << collisionShape->SignalMessage_OnAreaEntered << "\n\n\n\n";
            }

                      ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Collision Layers");

            ImGui::Text("Layer:");
            ImGui::SameLine();
            ImGui::PushItemWidth(100);

            uint8_t currentLayer = collisionShape->GetCollisionLayer();
            int selectedLayer = -1;

            for (int i = 0; i < 8; i++) {
                if (collisionShape->GetLayerBit(i)) {
                    selectedLayer = i;
                    break;
                }
            }

            std::string layerNames[8] = {"1", "2", "3", "4", "5", "6", "7", "8"};

            if (ImGui::BeginCombo("##LayerCombo", selectedLayer >= 0 ? layerNames[selectedLayer].c_str() : "None")) {
                for (int i = 0; i < 8; i++) {
                    bool isSelected = (selectedLayer == i);
                    if (ImGui::Selectable(layerNames[i].c_str(), isSelected)) {
                        collisionShape->SetCollisionLayer(1 << i);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();

            ImGui::Spacing();
            ImGui::Text("Mask:");
            for (int i = 0; i < 8; i++) {
                ImGui::PushID(i + 50);
                bool maskBit = collisionShape->GetMaskBit(i);
                if (ImGui::Checkbox(layerNames[i].c_str(), &maskBit)) {
                    collisionShape->SetMaskBit(i, maskBit);
                }
                ImGui::PopID();
                if (i == 3) {
                } else if (i < 7) {
                    ImGui::SameLine();
                }
            }

        } 
        else if (ThirdPersonCamera *tppCamera = dynamic_cast<ThirdPersonCamera *>(component)) {
            drawCameraPanel(tppCamera);
        }
        else if (Animator *pAnimator = dynamic_cast<Animator *>(component)) {
            drawAnimatorComponentPanel(pAnimator);
        } 
        else if (Camera *camera= dynamic_cast<Camera *>(component)) 
        {
            drawCameraPanel(camera);
        }
        else if (ParticleSystem* ps = dynamic_cast<ParticleSystem*>(component))
        {
            drawParticleSystemPanel(ps);
        } 
        else if (UI::Control *control = dynamic_cast<UI::Control *>(component)) 
        {
            drawControlPanel(control);
        } 
        else if (Timer *timer = dynamic_cast<Timer *>(component)){
            drawTimerPanel(timer);
        } else if (AISystem *aisystem = dynamic_cast<AISystem *>(component)) {
            drawAISystemPanel(aisystem);
        }
        else if (AIAgent *aiagent = dynamic_cast<AIAgent *>(component)) {
            drawAIAgentPanel(aiagent);
        }
        else if (NavigationMesh *navmesh = dynamic_cast<NavigationMesh *>(component)) {
            drawNavMeshPanel(navmesh);
        } 
        else if (NavigationTarget *target = dynamic_cast<NavigationTarget *>(component)) {
            ImGui::Text("Nothing to do here - providing target position for all agents");
        }
        else if (AudioSource* audioSource = dynamic_cast<AudioSource*>(component))
        {
            drawAudioSourcePanel(audioSource);
        }
        else if (Tag* pTag = dynamic_cast<Tag*>(component))
        {
            drawTagPanel(pTag);
        }
        else 
        {
            ImGui::Text("Unknown Component: %s", component->name.c_str());
        }

        ImGui::EndGroup();
        ImGui::Unindent();
    }

    if (toDelete)
    {
        gui.selectedGameObject->RemoveComponent(component);
    }

    float rightAlignX = ImGui::GetContentRegionMax().x - buttonSize.x;

}

void InspectorPanel::drawTextRendererComponentPanel(TextRenderer *textRenderer) {

    static char textBuff[256];
    std::strncpy(textBuff, textRenderer->text.c_str(), sizeof(textBuff - 1));
    ImGui::Text("Name:");
    ImGui::SameLine();
    if (ImGui::InputText("##textRendererTextInput", textBuff, sizeof(textBuff))) {
        textRenderer->text = textBuff;
    }

}

void InspectorPanel::drawLuaComponentPanel(LuaComponent *luaComponent) {
    EngineGUI &gui = EngineGUI::GetInstance();

    if (ImGuiFileDialog::Instance()->Display("LoadScript", NULL, ImVec2(500, 300)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string retPath = ImGuiFileDialog::Instance()->GetFilePathName();
            retPath = Util::getRelativePath(retPath).string();
            Util::FixPathString(retPath);

            luaComponent->SetScript(retPath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("CreateScript", NULL, ImVec2(500, 300)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string retPath = ImGuiFileDialog::Instance()->GetFilePathName();
            retPath = Util::getRelativePath(retPath).string();
            Util::FixPathString(retPath);

            Util::CreateTemplateScript(retPath);
            luaComponent->SetScript(retPath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Text("Script: ");
    ImGui::SameLine();
    ImGui::Text( !luaComponent->scriptPath.empty() ? luaComponent->scriptName.c_str() : "No script");

    if (ImGui::Button(ICON_FA_FILE_CODE_O " New script..."))
    {
        IGFD::FileDialogConfig config;
        config.path = "./res/scripts";
        ImGuiFileDialog::Instance()->OpenDialog("CreateScript", "Create script...", ".lua", config);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN " Load script..."))
    {
        IGFD::FileDialogConfig config;
        config.path = "./res/scripts";
        ImGuiFileDialog::Instance()->OpenDialog("LoadScript", "Load script...", ".lua", config);
    }

    if (!luaComponent->scriptPath.empty())
    {
        ImGui::SameLine();
        if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O " Remove script", Colors::DarkRed))
        {
            luaComponent->RemoveScript();
        }
    }
    else
    {
        return;
    }

    ImGui::Separator();

    if (ImGui::Button(ICON_FA_CODE " Edit Script")) {
        gui.scriptPath = luaComponent->scriptPath;
        gui.loadScript();
        gui.codeEditorEnabled = true;
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_RECYCLE " Reload Script"))
    {
        gui.scriptPath = luaComponent->scriptPath;
        gui.reloadCode(".lua");
    }

    std::vector<ExposedVar> &exposedVars = luaComponent->GetExposedVars();

    if (exposedVars.size() > 0) {
        
        if (Panel::CollapsingHeader("Export Variables")) {
            ImGui::Indent();

            for (auto &ev: exposedVars) {
                ImGui::PushID(ev.name.c_str());

                sol::type t = ev.defaultValue.get_type();
                ImGui::PushItemWidth(-1);

                // vec3
                if (ev.defaultValue.is<glm::vec3>()) {
                    glm::vec3 v = ev.defaultValue.as<glm::vec3>();

                    ImGui::BeginGroup();
                    ImGui::Text(ev.name.c_str());
                    ImGui::SameLine();

                    if (ImGui::DragFloat3("##export_vec3", (float *) &v, 0.1f)) {
                        ev.defaultValue = sol::make_object(luaComponent->L, v);
                        luaComponent->L[ev.name] = v; // update the Lua variable
                    }
                    ImGui::EndGroup();
                }
                // number
                else if (t == sol::type::number) {
                    float v = ev.defaultValue.as<float>();

                    ImGui::BeginGroup();

                    ImGui::Text(ev.name.c_str());
                    ImGui::SameLine();
                    if (ImGui::DragFloat("##export_float", &v)) {
                        ev.defaultValue = sol::make_object(luaComponent->L, v);

                        luaComponent->L[ev.name] = v; // update the Lua variable

                    }

                    ImGui::EndGroup();

                } 
                // boolean
                else if (t == sol::type::boolean) {
                    bool b = ev.defaultValue.as<bool>();

                    ImGui::BeginGroup();
                    ImGui::Text(ev.name.c_str());   
                    ImGui::SameLine();
                    if (ImGui::Checkbox("##export_bool", &b)) {
                        ev.defaultValue = sol::make_object(luaComponent->L, b);
                        luaComponent->L[ev.name] = b; // update the Lua variable
                    }
                    ImGui::EndGroup();

                } 
                // string
                else if (t == sol::type::string) {
                    std::string s = ev.defaultValue.as<std::string>();
                    char buf[256];
                    std::strncpy(buf, s.c_str(), sizeof(buf));

                    ImGui::BeginGroup();

                    ImGui::Text(ev.name.c_str());
                    ImGui::SameLine();
                    if (ImGui::InputText("##export_string", buf, sizeof(buf))) {
                        ev.defaultValue = sol::make_object(luaComponent->L, std::string(buf));
                        luaComponent->L[ev.name] = std::string(buf); // update the Lua variable
                    }

                    ImGui::EndGroup();
                } 

                ImGui::PopItemWidth();
                ImGui::PopID();
            }

            ImGui::Unindent();
        }
    
    }

    
}

void InspectorPanel::drawLightComponentPanel(Light *lightComponent) {

    float intensity = lightComponent->GetIntensity();

    ImGui::Text("Intensity: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##LightIntensityDrag", &intensity, 0.01f, 0.1f, 100.0f)) {
        lightComponent->SetIntensity(intensity);
    }
    ImGui::PopItemWidth();

    // edit diffuse
    glm::vec3 diffuse = lightComponent->GetDiffuse();

    ImGui::Text("Diffuse: ");
    ImGui::PushItemWidth(-1);
    if (ImGui::ColorEdit3("##LightDiffuseDrag", (float *) &diffuse)) {
        lightComponent->SetDiffuse(diffuse);
    }
    ImGui::PopItemWidth();

    // edit ambient
    glm::vec3 ambient = lightComponent->GetAmbient();

    ImGui::Text("Ambient:");
    ImGui::PushItemWidth(-1);
    if (ImGui::ColorEdit3("##LightAmbientDrag", (float *) &ambient)) {
        lightComponent->SetAmbient(ambient);
    }
    ImGui::PopItemWidth();

    float ambientIntensity = lightComponent->GetAmbientIntensity();

    ImGui::Text("Ambient Intensity: ");
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##LightAmbientIntensityDrag", &ambientIntensity, 0.001f, 0.001, 1.0f)) {
        lightComponent->SetAmbientIntensity(ambientIntensity);
    }
    ImGui::PopItemWidth();

    // edit specular
    glm::vec3 specular = lightComponent->GetSpecular();

    ImGui::Text("Specular:");
    ImGui::PushItemWidth(-1);
    if (ImGui::ColorEdit3("##LightSpecularDrag", (float *) &specular)) {
        lightComponent->SetSpecular(specular);
    }
    ImGui::PopItemWidth();

}

void InspectorPanel::drawLocalLightComponentPanel(LocalLight *localLightComponent) {

    float constant = localLightComponent->GetConstant();

    ImGui::Text("Constant: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##LightConstantDrag", &constant, 0.001f, 0.0f, 100.0f)) {
        localLightComponent->SetConstant(constant);
    }
    ImGui::PopItemWidth();
    float linear = localLightComponent->GetLinear();

    ImGui::Text("Linear: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##LightLinearDrag", &linear, 0.001f, 0.0f, 100.f)) {
        localLightComponent->SetLinear(linear);
    }
    ImGui::PopItemWidth();
    float quadratic = localLightComponent->GetQuadratic();

    ImGui::Text("Quadratic: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##LightQuadraticDrag", &quadratic, 0.001f, 0.0f, 100.0f)) {
        localLightComponent->SetQuadratic(quadratic);
    }
    ImGui::PopItemWidth();

    ImGui::Text("Cast Shadows: ");
    ImGui::SameLine();
    ImGui::Checkbox("##LightCastShadows", &localLightComponent->castShadows);

    // edit shadow distance

    ImGui::PushItemWidth(-1);
    ImGui::Text("Shadow Distance:");
    ImGui::SameLine();
    ImGui::DragFloat("##LightShadowDistance", &localLightComponent->shadowDistance, 1.0f, 0.1f, NULL, "%.2f");
    ImGui::PopItemWidth();

}

void InspectorPanel::drawSpotLightComponentPanel(SpotLight *spotLightComponent) {

    float innerCutOff = spotLightComponent->GetInnerCutOff();

    ImGui::Text("Inner Cut Off: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##LightInnerCutOff", &innerCutOff, 0.001f, 0.0f, 100.0f)) {
        spotLightComponent->SetInnerCutOff(innerCutOff);
    }
    ImGui::PopItemWidth();

    float outerCutOff = spotLightComponent->GetOuterCutOff();

    ImGui::Text("Outer Cut Off: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##LightOuterCutOff", &outerCutOff, 0.001f, 0.0f, 100.0f)) {
        spotLightComponent->SetOuterCutOff(outerCutOff);
    }
    ImGui::PopItemWidth();

}

void InspectorPanel::drawWorldEnvComponentPanel(WorldEnvironment *worldEnvComponent) {

    static std::vector<std::string> skyTypes = {"HDRI", "PHYSICAL"};

    static int selectedSkyType = worldEnvComponent->GetSkyType();

    ImGui::Text("Sky Type:");
    ImGui::PushItemWidth(-1);
    if (ImGui::BeginCombo("##SkyTypeCombo", skyTypes.at(selectedSkyType).c_str())) {
        for (int i = 0; i < skyTypes.size(); i++) {
            bool isSelected = (selectedSkyType == i);
            if (ImGui::Selectable(skyTypes[i].c_str(), isSelected)) {
                selectedSkyType = i;
                worldEnvComponent->SetSkyType(selectedSkyType);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    if (worldEnvComponent->GetSkyType() == 0){
        ImGui::DragFloat("IrradianceStrength", &worldEnvComponent->irradianceStrength, 0.01f, 0.00f, 1.0f);
        ImGui::DragFloat("SampleDelta", &worldEnvComponent->sampleDelta, 0.01f, 0.01f, 1.f);
    }
    ImGui::Text("Skybox Path:");
    ImGui::PushItemWidth(-1);
    static char pathBuffer[256] = "";
    static std::string lastPath = "";
    if (lastPath != worldEnvComponent->m_skyboxPath) {
        strncpy(pathBuffer, worldEnvComponent->m_skyboxPath.c_str(), sizeof(pathBuffer) - 1);
        pathBuffer[sizeof(pathBuffer) - 1] = '\0';
        lastPath = worldEnvComponent->m_skyboxPath;
    }
    if (ImGui::InputText("##SkyboxPath", pathBuffer, sizeof(pathBuffer))) {

    }
    ImGui::PopItemWidth();
    if (ImGui::Button("Confirm / Reload")) {
        std::string newPath = std::string(pathBuffer);
        if (!newPath.empty() && Util::fileExists(newPath)) {
            worldEnvComponent->m_skyboxPath = newPath;
            worldEnvComponent->LoadHDRI();
        }
    }
}

void InspectorPanel::drawMeshInstanceComponentPanel(MeshInstance *meshInstanceComponent) {
    ImGui::Checkbox("Use Volumetric", &meshInstanceComponent->m_useVolumetric);
    if (meshInstanceComponent->isUsingVolumetric()) {
        if (Panel::CollapsingHeader("Volumetric Mesh")) {
            ImGui::SliderFloat("Density", &meshInstanceComponent->density, -0.99f, 2.0f);
            ImGui::SliderInt("Samples", &meshInstanceComponent->samples, 1, 512);
            ImGui::ColorEdit3("Fog Color", (float *) &meshInstanceComponent->fogColor);
            ImGui::SliderFloat("Scattering", &meshInstanceComponent->scattering, -5.0f, 10.0f);
        }
    }
    auto model = meshInstanceComponent->model;
    if (Panel::CollapsingHeader("Mesh"))
    {
        if (model)
        {
            ImGui::Text("Loaded mesh: ");
            ImGui::SameLine();
            ImGui::Text(model->Name.c_str());
        }
        else
        {
            ImGui::Text("No mesh");
        }

        if (ImGui::Button(ICON_FA_FOLDER_OPEN " Choose mesh..."))
        {
            IGFD::FileDialogConfig config;
            config.path = "./res/models";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseMesh", "Choose mesh...", "Mesh files (*.fbx *.obj *.dae){.fbx,.obj,.dae}", config);
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PATH"))
            {
                if (payload->DataSize > 0)
                {
                    std::string pathStr = std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
                    pathStr = Util::getRelativePath(pathStr).string();

                    Util::FixPathString(pathStr);
                    meshInstanceComponent->model = ResourceManager::GetInstance().LoadMeshFromFile(pathStr);
                    meshInstanceComponent->m_modelPath = pathStr;
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    if (model && !meshInstanceComponent->isUsingVolumetric())
    {

        m_MaterialPanel.SetMaterial(&model->Material);
        m_MaterialPanel.SetMeshInstance(meshInstanceComponent);
        m_MaterialPanel.Draw();
    } 
    ImGui::Separator();
    
    if (ImGuiFileDialog::Instance()->Display("ChooseMesh", NULL, ImVec2(500, 300)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string retPath = ImGuiFileDialog::Instance()->GetFilePathName();
            retPath = Util::getRelativePath(retPath).string();
            Util::FixPathString(retPath);

            meshInstanceComponent->model = ResourceManager::GetInstance().LoadMeshFromFile(retPath);
            meshInstanceComponent->m_modelPath = retPath;
        }
        ImGuiFileDialog::Instance()->Close();
    }

}

void InspectorPanel::drawAnimatorComponentPanel(Animator *pAnimator) {
    EngineGUI &gui = EngineGUI::GetInstance();

    bool isPlaying = pAnimator->IsPlaying();
    static bool pAnimatorPaused = false;

    std::vector<std::string> animations = pAnimator->GetAllAnimationsNames();
    static int chosenAnimation = -1;
    chosenAnimation = pAnimator->GetIndexOfCurrentAnimation();

    ImGui::Text("Current Animation");
    if (ImGui::BeginCombo("##animations_combo",
                          chosenAnimation == -1 ? "Not Selected" : animations.at(chosenAnimation).c_str())) {

        if (ImGui::Selectable("Not Selected", chosenAnimation == -1)) {
            chosenAnimation = -1;
            pAnimator->DeselectAnimation();
        }

        if (animations.size() > 0) {
            ImGui::SeparatorText("Animations");
        }

        for (int i = 0; i < animations.size(); i++) {
            bool isSelected = (chosenAnimation == i);

            if (ImGui::Selectable(animations.at(i).c_str(), isSelected)) {
                chosenAnimation = i;
                pAnimator->SetCurrentAnimation(chosenAnimation);
            }
        }

        ImGui::EndCombo();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Animation_DND")) {
            if (payload->DataSize > 0) {
                std::string key = std::string(static_cast<const char *>(payload->Data), payload->DataSize - 1);
                if (auto foundAnim = AnimationLibrary::GetInstance().GetAnimation(key)) {
                    pAnimator->AddAnimation(foundAnim);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (auto pAnimation = pAnimator->GetCurrentAnimation()) {
        float progress = pAnimator->GetCurrentPlaybackTime() / pAnimation->GetDuration();
        progress = std::clamp(progress, 0.0f, 1.0f);

        float currentTime = pAnimator->GetCurrentPlaybackTime();
        float duration = pAnimation->GetDuration();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 3));
        ImGui::PushItemWidth(-1);
        // anim progress bar
        if (ImGui::SliderFloat("##AnimTime", &currentTime, 0.0f, duration, "")) {
            pAnimator->Play();
            pAnimator->SetCurrentPlaybackTime(currentTime);
        }
        if (ImGui::IsItemDeactivated()) {
            if (pAnimatorPaused)
                pAnimator->Pause();
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(-1);
        float currTimeSec = currentTime / pAnimation->GetTicksPerSecond();
        float durTimeSec = duration / pAnimation->GetTicksPerSecond();
        int currMinutes = int(currTimeSec / 60.0f);
        int currSeconds = int(currTimeSec) % 60;
        int currHundredths = int((currTimeSec - floor(currTimeSec)) * 100.0f);

        int durMinutes = int(durTimeSec / 60.0f);
        int durSeconds = int(durTimeSec) % 60;
        int durHundredths = int((durTimeSec - floor(durTimeSec)) * 100.0f);
        ImGui::Text("%02d:%02d.%02d - %02d:%02d.%02d", currMinutes, currSeconds, currHundredths, durMinutes, durSeconds,
                    durHundredths);
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        ImGui::PushItemWidth(-1);

        ImGui::Text("Time Scale ");
        ImGui::SameLine();
        float timeScale = pAnimator->GetTimeScale();
        if (ImGui::DragFloat("##AnimTimeScale", &timeScale, 0.01f, 0.01f, 10.0f)) {
            pAnimator->SetTimeScale(timeScale);
        }

        ImGui::PopItemWidth();

        /*bool blendingEnabled = pAnimator->IsBlending();
        if (ImGui::Checkbox("Blending", &blendingEnabled)) {
            pAnimator->SetBlending(blendingEnabled);
        }

        if (blendingEnabled) {

            static int chosenLayeredAnimation = -1;
            chosenLayeredAnimation = pAnimator->GetIndexOfLayeredAnimation();

            ImGui::Text("Layered Animation");
            ImGui::PushItemWidth(-1);
            if (ImGui::BeginCombo("##animations_combo_layered",
                                  chosenLayeredAnimation == -1
                                          ? "Not Selected"
                                          : animations.at(chosenLayeredAnimation).c_str())) {

                if (ImGui::Selectable("Not Selected", chosenLayeredAnimation == -1)) {
                    chosenLayeredAnimation = -1;
                    pAnimator->DeselectLayeredAnimation();
                }

                if (animations.size() > 0) {
                    ImGui::SeparatorText("Animations");
                }

                for (int i = 0; i < animations.size(); i++) {
                    bool isSelected = (chosenLayeredAnimation == i);

                    if (ImGui::Selectable(animations.at(i).c_str(), isSelected)) {
                        chosenLayeredAnimation = i;
                        pAnimator->SetLayeredAnimation(animations.at(chosenLayeredAnimation));
                    }
                }

                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();

            ImGui::PushItemWidth(-1);

            ImGui::Text("Blend Factor ");
            ImGui::SameLine();
            float blendFactor = pAnimator->GetBlendFactor();
            if (ImGui::DragFloat("##AnimBlendFactor", &blendFactor, 0.01f, 0.00f, 1.0f)) {
                pAnimator->SetBlendFactor(blendFactor);
            }

            ImGui::PopItemWidth();
        }*/

        // buttons
        float width = 100.0f;
        // Panel::CenterWidget(width);

        ImGui::PushItemWidth(-1);
        ImGui::BeginGroup();
        if (Panel::ColoredButton(ICON_FA_PLAY, isPlaying ? Colors::DarkGreen : Colors::Gray, ImVec2(width / 2, 0))) {
            pAnimator->Play();
            pAnimatorPaused = false;
        }
        ImGui::SameLine();
        if (Panel::ColoredButton(ICON_FA_PAUSE, isPlaying ? Colors::Gray : Colors::DarkGreen, ImVec2(width / 2, 0))) {
            pAnimator->Pause();
            pAnimatorPaused = true;
        }
        ImGui::EndGroup();
        ImGui::PopItemWidth();

        ImGui::SeparatorText("Transitions");

        if (ImGui::Button(ICON_FA_EXCHANGE " Open Transitions Panel")) {
            gui.GetAnimationTransitionPanel().SetAnimator(pAnimator);
            gui.GetAnimationTransitionPanel().Open();
        }
    }


}

void InspectorPanel::drawCameraPanel(Camera *camera) {

    EngineGUI &gui = EngineGUI::GetInstance();
    
    if (Panel::ColoredButton(ICON_FA_EYE "Use Lerp", this->useLerp ? Colors::DarkGreen : Colors::Gray)) {
        useLerp = !useLerp;
    }
    ImGui::SameLine();
    if (Panel::ColoredButton(ICON_FA_CAMERA "Use Camera", camera->isUsed ? Colors::DarkGreen : Colors::Gray)) {
        if (camera->isUsed && !camera->isLerping){
            camera->isUsed = false;
            Signals::Engine_ReturnToSceneCamera.emit(useLerp);
        } 
        else if (!camera->isLerping) {
            camera->isUsed = true;
            Signals::Engine_UseCamera.emit(camera->gameObject->uuid, useLerp);
        }
    }
}

void InspectorPanel::drawParticleSystemPanel(ParticleSystem *pParticleSystem) 
{

    ImGui::Text("Emitting: ");
    ImGui::SameLine();
    ImGui::Checkbox("##ParticleSystemEmitting", &pParticleSystem->Emitting);
    
    ImGui::Text("One Shot: ");
    ImGui::SameLine();
    ImGui::Checkbox("##ParticleSystemOneShot", &pParticleSystem->OneShot);

    ImGui::Text("Max Particles: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::DragInt("##ParticleSystemMaxParticles", (int*) & pParticleSystem->MaxParticles, 1.0f, 1, 10000)) {
        pParticleSystem->SetMaxParticles(pParticleSystem->MaxParticles);
    }
    ImGui::PopItemWidth();

    ImGui::Text("Spawn Rate: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    ImGui::DragFloat("##ParticleSystemSpawnRate", &pParticleSystem->SpawnRate, 0.01f, 0.0f);
    ImGui::PopItemWidth();

    if (ImGui::TreeNode("Emitter"))
    {
        static std::vector<std::string> emissionShapes = {"Point", "Sphere", "Cube"};
        static int selectedEmissionShape = (int) pParticleSystem->EmissionShape;
        ImGui::Text("Emission Shape:");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (ImGui::BeginCombo("##EmissionShapeCombo", emissionShapes.at(selectedEmissionShape).c_str())) {
            for (int i = 0; i < emissionShapes.size(); i++) {
                bool isSelected = (selectedEmissionShape == i);
                if (ImGui::Selectable(emissionShapes[i].c_str(), isSelected)) {
                    selectedEmissionShape = i;
                    pParticleSystem->EmissionShape = (EmissionShapeType) selectedEmissionShape;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();

        ImGui::Text("Draw Emission Shape: ");
        ImGui::SameLine();
        ImGui::Checkbox("##DrawEmissionShape", &pParticleSystem->DrawEmissionShape);

        switch ((EmissionShapeType)selectedEmissionShape)
        {
            case EmissionShapeType::Point:
                break;
            case EmissionShapeType::Cube:
                Panel::DragVec3Widget("Cube Dimensions", pParticleSystem->CubeSize, 1.0f);
                break;
            case EmissionShapeType::Sphere:
                ImGui::Text("Radius: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##ParticleSystemSphereRadius", &pParticleSystem->SphereRadius, 0.01f, 0.0f); 
                ImGui::PopItemWidth();
                ImGui::Text("Surface Only: ");
                ImGui::SameLine();
                ImGui::Checkbox("##SurfaceOnlyCheck", &pParticleSystem->SphereSurfaceOnly);
                break;
        }

        ImGui::Text("Use Local Space: ");
        ImGui::SameLine();
        ImGui::Checkbox("##UseLocalSpace", &pParticleSystem->UseLocalSpace);

        ImGui::TreePop();
    }

    

    if (ImGui::TreeNode("Display"))
    {
        ImGui::Text("Texture: ");
        ImGui::SameLine();
        std::shared_ptr<Texture> texture = pParticleSystem->GetTexture();
        if (ImGui::ImageButton("##TextureParticleEdit", (ImTextureID) (texture ? texture->id : 0), ImVec2(32, 32),
                               ImVec2(0, 1), ImVec2(1, 0))) {
            // not
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_PATH")) {
                if (payload->DataSize > 0) {
                    // Convert payload data to string
                    std::string texturePath =
                            std::string(static_cast<const char *>(payload->Data), payload->DataSize - 1);
                    std::shared_ptr<Texture> texture = ResourceManager::GetInstance().LoadTextureFromFile(texturePath.c_str());
                    pParticleSystem->SetTexture(texture);
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Text("Albedo: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::ColorEdit4("##ParticleSystemAlbedo", (float *) &pParticleSystem->Albedo);
        ImGui::PopItemWidth();

        ImGui::Text("Emission: ");
        ImGui::SameLine();
        ImGui::DragFloat("##ParticleEmission", (float *) &pParticleSystem->EmissionMultiplier, 1.0f);

        ImGui::Text("Additive Blend ");
        ImGui::SameLine();
        ImGui::Checkbox("##ParticleAddBlend", &pParticleSystem->BlendingAdditive);

        ImGui::Text("Billboard Only Y: ");
        ImGui::SameLine();
        ImGui::Checkbox("##ParticleBillboardOnlyY", &pParticleSystem->BillboardY);

        ImGui::TreePop();
    }
    

    if (ImGui::TreeNode("Life Time"))
    {
        Panel::DragMinMaxWidget("", pParticleSystem->MinLifeTime, pParticleSystem->MaxLifeTime,
                                1.0f, 50.0f);
        ImGui::NewLine();
        ImGui::TreePop();

        ImGui::Indent();

        ImGui::Text("Scale Over Time: ");
        ImGui::SameLine();
        ImGui::Checkbox("##ScaleOverTimeChbvkx", &pParticleSystem->ScaleOverTime);

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Particles will scale down over time.");
        }


        if (pParticleSystem->ScaleOverTime) {
            ImGui::Text("Begin Scaling: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##ParticleSystemScaleFactor", &pParticleSystem->BeginScaling, 0.01f, 0.0f, 1.0f);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("The normalized lifetime (0-1) when particles start scaling.");
            }
            ImGui::PopItemWidth();
        }

        ImGui::Text("Fade Over Time: ");
        ImGui::SameLine();
        ImGui::Checkbox("##FadeOverTimeChbvkx", &pParticleSystem->FadeOverTime);

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Particles will fade out over time.");
        }


        if (pParticleSystem->FadeOverTime)
        {
            ImGui::Text("Begin Fading: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##ParticleSystemFadeFactor", &pParticleSystem->BeginFading, 0.01f, 0.0f, 1.0f);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("The normalized lifetime (0-1) when particles start fading out.");
            }
            ImGui::PopItemWidth();
        }

        ImGui::Unindent();
    }

    if (ImGui::TreeNode("Size"))
    {
        Panel::DragMinMaxWidget("", pParticleSystem->MinSize, pParticleSystem->MaxSize, 1.0f, 50.0f);
        ImGui::NewLine();
        ImGui::TreePop();
    }

    ImGui::SeparatorText("Forces");

    Panel::DragVec3Widget("Gravity: ", pParticleSystem->Gravity, 0.0f);

    ImGui::Spacing();

    if (ImGui::TreeNode("Initial Velocity")) {
        Panel::DragVec3MinMaxWidget("", pParticleSystem->MinInitialVelocity, pParticleSystem->MaxInitialVelocity,
                                    1.0f, 100.0f);
        ImGui::NewLine();
        ImGui::TreePop();
    }


    
}

void InspectorPanel::drawControlPanel(UI::Control* pControl)
{
    const char* anchorNames[] = {
            "TopLeft", "TopCenter", "TopRight",
            "MiddleLeft", "MiddleCenter", "MiddleRight",
            "BottomLeft", "BottomCenter", "BottomRight"
    };
    int currentAnchor = static_cast<int>(pControl->anchor);
    ImGui::Text("Anchor: ");
    ImGui::SameLine();
    if (ImGui::Combo("##Anchor", &currentAnchor, anchorNames, IM_ARRAYSIZE(anchorNames)))
    {
        pControl->anchor = static_cast<UI::Anchor>(currentAnchor);
    }

    Panel::DragVec2Widget("Position", pControl->Position, 0.0f, 100.0f);
    ImGui::NewLine();
    Panel::DragVec2Widget("Size", pControl->Size, 100.0f, 100.0f);
    ImGui::NewLine();
    
    ImGui::Text("Rotation: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    ImGui::DragFloat("##RotationControl", &pControl->Rotation, 0.1f, 0.0f, 360.0f);
    ImGui::PopItemWidth();
   

    if (auto pButton = dynamic_cast<UI::Button*>(pControl))
    {
        ImGui::Text("Color: ");
        ImGui::PushItemWidth(-1);
        ImGui::ColorEdit3("##ButtonColorEdit", (float*) &pButton->Color);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(-1);
        ImGui::Text("Emission:");
        ImGui::SameLine();
        ImGui::DragFloat("##ButtonEmissionMlp", &pButton->Emission, 0.1f, 0.0f, 100.0f);
        ImGui::PopItemWidth();

        if (ImGui::TreeNode("Texture"))
        {
            ImGui::Text("Default: ");
            ImGui::SameLine();
            ImGui::ImageButton("##ButtonTexture", (ImTextureID)pButton->GetTexture()->id, ImVec2(32,32));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PATH"))
                {
                    if (payload->DataSize > 0)
                    {
                        std::string path = std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1);

                        std::shared_ptr<Texture> texture = ResourceManager::GetInstance().LoadTextureFromFile(path);
                        pButton->SetTexture(texture);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (pButton->HasTexture())
            {
                ImGui::PushID("RemoveTexture");
                ImGui::SameLine();

                if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32)))
                {
                    pButton->RemoveTexture();
                }
                ImGui::PopID();
            }

            ImGui::Text("Hovered: ");
            ImGui::SameLine();
            ImGui::ImageButton("##ButtonHoveredTexture", (ImTextureID)pButton->GetHoverTexture()->id, ImVec2(32, 32));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PATH"))
                {
                    if (payload->DataSize > 0)
                    {
                        std::string path = std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1);

                        std::shared_ptr<Texture> texture = ResourceManager::GetInstance().LoadTextureFromFile(path);
                        pButton->SetHoverTexture(texture);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (pButton->GetHoverTexture()->id != 0)
            {
                ImGui::SameLine();
                ImGui::PushID("RemoveHoverTexture");
                if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32)))
                {
                    pButton->RemoveHoverTexture();
                }
                ImGui::PopID();
            }

            ImGui::TreePop();
        }
    } 
    else if (auto pText = dynamic_cast<UI::Text *>(pControl)) {
        static char textBuffer[512] = "";
        static UI::Text *lastText = nullptr;

        if (pText != lastText && pText != nullptr) {
            strncpy(textBuffer, pText->GetText().c_str(), sizeof(textBuffer) - 1);
            textBuffer[sizeof(textBuffer) - 1] = '\0';
            lastText = pText;
        }

        ImGui::Text("Text Content:");
        ImGui::PushItemWidth(-1);
        if (ImGui::InputTextMultiline("##TextContent", textBuffer, sizeof(textBuffer), ImVec2(-1, 60))) {
            pText->SetText(std::string(textBuffer));
        }
        ImGui::PopItemWidth();

        float fontSize = pText->GetFontSize();
        ImGui::Text("Font Size:");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (ImGui::DragFloat("##TextFontSize", &fontSize, 0.5f, 8.0f, 128.0f)) {
            pText->SetFontSize(fontSize);
        }
        ImGui::PopItemWidth();

        glm::vec3 textColor = pText->GetColor();
        ImGui::Text("Text Color:");
        ImGui::PushItemWidth(-1);
        if (ImGui::ColorEdit3("##TextColorEdit", (float *) &textColor)) {
            pText->SetColor(textColor);
        }
        ImGui::PopItemWidth();

        ImGui::Text("Emission:");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::DragFloat("##TextEmission", &pText->Emission, 0.1f, 0.0f, 10.0f);
        ImGui::PopItemWidth();
    }
    else if (auto pSprite = dynamic_cast<UI::Sprite*>(pControl))
    {
        ImGui::Text("Modulate: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        ImGui::ColorEdit4("##SpriteColorEdit", (float *) &pSprite->ModulateColor);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(-1);
        ImGui::Text("Emission:");
        ImGui::SameLine();
        ImGui::DragFloat("##SpriteEmissionMlp", &pSprite->Emission, 0.1f, 0.0f, 100.0f);
        ImGui::PopItemWidth();

        if (ImGui::TreeNode("Texture"))
        {
            ImGui::Text("Texture: ");
            ImGui::SameLine();
            ImGui::ImageButton("##SpriteTexture", (ImTextureID)pSprite->GetTexture()->id, ImVec2(32, 32));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PATH"))
                {
                    if (payload->DataSize > 0)
                    {
                        std::string path = std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1);

                        std::shared_ptr<Texture> texture = ResourceManager::GetInstance().LoadTextureFromFile(path);
                        pSprite->SetTexture(texture);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (pSprite->HasTexture())
            {
                ImGui::SameLine();

                if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32)))
                {
                    pSprite->RemoveTexture();
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Clipping"))
        {
            // Clipping Left
            ImGui::Text("Left: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##SpriteClipLeft", &pSprite->Clipping.x, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::PopItemWidth();

            // Clipping Right
            ImGui::Text("Right: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##SpriteClipRight", &pSprite->Clipping.y, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::PopItemWidth();

            // Clipping Top
            ImGui::Text("Top: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##SpriteClipTop", &pSprite->Clipping.z, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::PopItemWidth();

            // Clipping Bottom
            ImGui::Text("Bottom: ");
            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            ImGui::DragFloat("##SpriteClipBottom", &pSprite->Clipping.w, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::PopItemWidth();



            ImGui::TreePop();
        }
    }
}

void InspectorPanel::drawTimerPanel(Timer *pTimer) {


    ImGui::Checkbox("##Started", &pTimer->HasStarted);
    ImGui::SameLine();
    ImGui::Text("Started");
    ImGui::Checkbox("##TimerEmitting", &pTimer->OneShot);
    ImGui::SameLine();
    ImGui::Text("OneShot");
    
    Panel::DragFloatWidget("Timeout Ms", pTimer->TimeoutMs);

}

void InspectorPanel::drawAudioSourcePanel(AudioSource *audioSource) 
{ 
    static float audioVolume = audioSource->GetVolumeFromAll() ? audioSource->GetVolumeFromAll() : 1.0;
    if (ImGui::DragFloat("Group Volume##Volume", &audioVolume, 0.01f, 0.0f, 10.0f, "%.2f")) {
        audioSource->SetVolumeAll(audioVolume);
    }
    static float audioPitch = audioSource->GetPitchFromAll() ? audioSource->GetPitchFromAll() : 1.0;
    if (ImGui::DragFloat("Group Pitch##Pitch", &audioPitch, 0.001f, 0.0f, 5.0f, "%.3f")) {
        audioSource->SetPitchAll(audioPitch);
    }
    
    if (ImGui::Button("PrintAllEvents")) {
        for(std::string_view str: audioSource->eventNames){
            EngineDebug::GetInstance().PrintDebug(str.data());
        }
    }

    if (ImGui::Button("Resume All")) {
        audioSource->ResumeAll();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause All")) {
        audioSource->PauseAll();
    }

    
    
    if (ImGui::Button("Play All")) {
        audioSource->PlayAll();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop All"))
    {
        audioSource->StopAll();
    }
    
}

void InspectorPanel::drawTagPanel(Tag *pTag) {
    static char textBuffer[256] = "";
    static Tag *lastTag = nullptr;

    // Check if pTag has changed (i.e., switched GameObject)
    if (pTag != lastTag && pTag != nullptr) {
        // Update textBuffer with the new pTag->Name
        strncpy(textBuffer, pTag->Name.c_str(), sizeof(textBuffer) - 1);
        textBuffer[sizeof(textBuffer) - 1] = '\0'; // Ensure null-termination
        lastTag = pTag;
    }
    ImGui::Text("Name: ");
    ImGui::SameLine();

    ImGui::PushItemWidth(-1);
    // Create a text input field
    if (ImGui::InputText("Enter text", textBuffer, sizeof(textBuffer))) {
        pTag->Name = std::string(textBuffer);
    }
    ImGui::PopItemWidth();
}

void InspectorPanel::drawAISystemPanel(AISystem *system) {
    if (!system)
        return;

    EngineGUI &gui = EngineGUI::GetInstance();

    // NAVIGATION MESH
    if (ImGui::TreeNode("Navigation Mesh")) {
        ImGui::Text("Navigation Mesh: ");

        NavigationMesh *navMesh = system->GetNavigationMesh();
        if (navMesh && navMesh->gameObject) {
            std::string navMeshName = navMesh->gameObject->name;
            ImGui::Button(navMeshName.c_str(), ImVec2(200, 32));
        } else {
            ImGui::Button("None", ImVec2(200, 32));
        }

        ImGui::SameLine();
        if (ImGui::Button("Select", ImVec2(75, 32))) {
            ImGui::OpenPopup("SelectNavMeshPopup");
        }

        if (ImGui::BeginPopupModal("SelectNavMeshPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select a GameObject with NavigationMesh:");
            ImGui::Separator();

            std::function<void(GameObject *)> drawSelectableNavMesh = [&](GameObject *go) {
                if (!go)
                    return;
                ImGui::PushID(go->GetUUID().c_str());

                NavigationMesh *candidate = go->GetComponent<NavigationMesh>();
                bool hasChildren = !go->children.empty();
                ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (!hasChildren)
                    treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                bool open = ImGui::TreeNodeEx(go->name.c_str(), treeFlags);
                if (candidate && ImGui::IsItemClicked()) {
                    system->SetNavigationMesh(candidate);
                    EngineDebug::GetInstance().PrintInfo("AISYSTEM: Navigation Mesh set to " + go->name);
                    ImGui::CloseCurrentPopup();
                    ImGui::PopID();
                    return;
                }

                if (open && hasChildren) {
                    for (auto &child: go->children)
                        drawSelectableNavMesh(child.get());
                    ImGui::TreePop();
                }

                ImGui::PopID();
            };

            drawSelectableNavMesh(gui.GetScene()->sceneRootObject.get());

            ImGui::Separator();
            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        // Add remove button if navmesh is set
        if (navMesh) {
            ImGui::SameLine();
            if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32))) {
                system->SetNavigationMesh(nullptr);
                EngineDebug::GetInstance().PrintInfo("AISYSTEM: Navigation Mesh set to nullptr");
            }
        }

        ImGui::TreePop();
    }

    // AGENTS
    if (ImGui::TreeNode("Agents")) {
        ImGui::Text("Registered Agents: ");

        auto agents = system->GetAgents();
        int index = 0; // For ImGui::PushID uniqueness
        for (auto agent: agents) {
            std::string agentName = agent && agent->gameObject ? agent->gameObject->name : "None";

            ImGui::PushID(index++);

            ImGui::Button(agentName.c_str(), ImVec2(200, 32));

            // Add remove button for each agent
            if (agent) {
                ImGui::SameLine();
                if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32))) {
                    system->RemoveAgent(agent);
                    EngineDebug::GetInstance().PrintInfo("AISYSTEM: Agent removed from system");
                }
            }

            ImGui::PopID();
        }

        // Add Agent button
        ImGui::PushID("AddAgent");
        if (ImGui::Button("Add Agent", ImVec2(200, 32))) {
            ImGui::OpenPopup("AddAgentPopup");
        }

        if (ImGui::BeginPopupModal("AddAgentPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select a GameObject with AIAgent:");
            ImGui::Separator();

            std::function<void(GameObject *)> drawSelectableAgent = [&](GameObject *go) {
                if (!go)
                    return;
                ImGui::PushID(go->GetUUID().c_str());

                AIAgent *candidate = go->GetComponent<AIAgent>();
                bool hasChildren = !go->children.empty();
                ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (!hasChildren)
                    treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                bool open = ImGui::TreeNodeEx(go->name.c_str(), treeFlags);
                if (candidate && ImGui::IsItemClicked()) {
                    system->RegisterAgent(candidate);
                    EngineDebug::GetInstance().PrintInfo("AISYSTEM: Agent " + go->name + " registered to system");
                    ImGui::CloseCurrentPopup();
                    ImGui::PopID();
                    return;
                }

                if (open && hasChildren) {
                    for (auto &child: go->children)
                        drawSelectableAgent(child.get());
                    ImGui::TreePop();
                }

                ImGui::PopID();
            };

            drawSelectableAgent(gui.GetScene()->sceneRootObject.get());

            ImGui::Separator();
            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::PopID();
        ImGui::TreePop();
    }

    // NAVIGATION TARGET
    if (ImGui::TreeNode("Navigation Target")) {
        ImGui::Text("Target: ");
        ImGui::SameLine();

        NavigationTarget *target = system->GetTarget();
        if (target && target->gameObject) {
            std::string targetName = target->gameObject->name;
            ImGui::Button(targetName.c_str(), ImVec2(200, 32));
        } else {
            ImGui::Button("None", ImVec2(200, 32));
        }

        ImGui::SameLine();
        if (ImGui::Button("Select", ImVec2(75, 32))) {
            ImGui::OpenPopup("SelectTargetPopup");
        }

        if (ImGui::BeginPopupModal("SelectTargetPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select a GameObject with NavigationTarget:");
            ImGui::Separator();

            std::function<void(GameObject *)> drawSelectableTarget = [&](GameObject *go) {
                if (!go)
                    return;
                ImGui::PushID(go->GetUUID().c_str());

                NavigationTarget *candidate = go->GetComponent<NavigationTarget>();
                bool hasChildren = !go->children.empty();
                ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (!hasChildren)
                    treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                bool open = ImGui::TreeNodeEx(go->name.c_str(), treeFlags);
                if (candidate && ImGui::IsItemClicked()) {
                    system->SetTarget(candidate);
                    EngineDebug::GetInstance().PrintInfo("AISYSTEM: Navigation Target set to " + go->name);
                    ImGui::CloseCurrentPopup();
                    ImGui::PopID();
                    return;
                }

                if (open && hasChildren) {
                    for (auto &child: go->children)
                        drawSelectableTarget(child.get());
                    ImGui::TreePop();
                }

                ImGui::PopID();
            };

            drawSelectableTarget(gui.GetScene()->sceneRootObject.get());

            ImGui::Separator();
            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        // Add remove button if target is set
        if (target) {
            ImGui::SameLine();
            if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32))) {
                system->SetTarget(nullptr);
                EngineDebug::GetInstance().PrintInfo("AISYSTEM: Navigation Target set to nullptr");
            }
        }

        ImGui::TreePop();
    }

    // SYSTEM CONTROLS
    if (ImGui::TreeNode("System Controls")) {
        // System-wide controls
        if (ImGui::Button("Start All Agents", ImVec2(150, 32))) {
            // Assuming there's a method to start all agents
            for (auto agent: system->GetAgents()) {
                if (agent && agent->IsPaused()) {
                    agent->Resume();
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Pause All Agents", ImVec2(150, 32))) {
            // Assuming there's a method to pause all agents
            for (auto agent: system->GetAgents()) {
                if (agent && !agent->IsPaused()) {
                    agent->Pause();
                }
            }
        }

        ImGui::TreePop();
    }
}

void InspectorPanel::drawAIAgentPanel(AIAgent *agent) {
    if (!agent)
        return;

    const char *stateNames[] = {"IDLE", "SEEK", "FLEE", "WANDER", "PATH_FOLLOWING", "ATTACK", "CIRCLING"};
    int currentState = static_cast<int>(agent->GetState());
    if (ImGui::Combo("State", &currentState, stateNames, IM_ARRAYSIZE(stateNames))) {
        agent->SetState(static_cast<AIAgentState>(currentState));
    }

    float speed = agent->GetSpeed();
    float maxSpeed = agent->GetMaxSpeed();
    float maxForce = agent->GetMaxForce();
    float mass = agent->GetMass();

    if (ImGui::InputFloat("Speed", &speed, 0.0f, 50.0f)) {
        agent->SetSpeed(speed);
    }
    if (ImGui::InputFloat("Max Speed", &maxSpeed, 0.0f, 100.0f)) {
        agent->SetMaxSpeed(maxSpeed);
    }
    if (ImGui::InputFloat("Max Force", &maxForce, 0.0f, 50.0f)) {
        agent->SetMaxForce(maxForce);
    }
    if (ImGui::InputFloat("Mass", &mass, 0.1f, 100.0f)) {
        agent->SetMass(mass);
    }

    // Stopping distance
    float stopping = agent->GetStoppingDistance();
    if (ImGui::InputFloat("Stopping Distance", &stopping, 0.0f, 10.0f)) {
        agent->SetStoppingDistance(stopping);
    }

    // Line of Sight
    float los = agent->GetLineOfSightDistance();
    if (ImGui::InputFloat("Line of Sight", &los, 0.0f, 100.0f)) {
        agent->SetLineOfSightDistance(los);
    }

    // Pause/Resume
    if (agent->IsPaused()) {
        if (ImGui::Button("Resume")) {
            agent->Resume();
        }
    } else {
        if (ImGui::Button("Pause")) {
            agent->Pause();
        }
    }
}


void InspectorPanel::drawNavMeshPanel(NavigationMesh *navmesh) {
    EngineGUI &gui = EngineGUI::GetInstance();

    if (ImGui::Button("Generate navigation mesh")) {
        navmesh->Generate();
    }

    if (ImGui::TreeNode("Floor ")) {
        ImGui::Text("Floor: ");
        ImGui::SameLine();

        if (navmesh->GetFloor() != nullptr) {
            std::string as = navmesh->gameObject->name;
            ImGui::Button(as.c_str(), ImVec2(200, 32));
        } else {
            ImGui::Button("None", ImVec2(200, 32));
        }

        ImGui::SameLine();
        if (ImGui::Button("Select", ImVec2(75, 32))) {
            ImGui::OpenPopup("Select Floor Mesh");
        }

        if (ImGui::BeginPopupModal("Select Floor Mesh", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select a GameObject with MeshInstance:");
            ImGui::Separator();

            std::function<void(GameObject *)> drawSelectableGameObject = [&](GameObject *go) {
                if (!go)
                    return;

                ImGui::PushID(go->GetUUID().c_str());

                MeshInstance *mesh = go->GetComponent<MeshInstance>();
                bool hasChildren = !go->children.empty();
                ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (!hasChildren)
                    treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                bool open = ImGui::TreeNodeEx(go->name.c_str(), treeFlags);

                if (mesh && ImGui::IsItemClicked()) {
                    navmesh->SetFloor(mesh);
                    EngineDebug::GetInstance().PrintInfo("NAVIGATIONMESH: Floor set to " + go->name);
                    ImGui::CloseCurrentPopup();
                    ImGui::PopID();
                    return;
                }

                if (open && hasChildren) {
                    for (const auto &child: go->children) {
                        drawSelectableGameObject(child.get());
                    }
                    ImGui::TreePop();
                }

                ImGui::PopID();
            };

            const auto root = gui.GetScene()->sceneRootObject.get();
            drawSelectableGameObject(root);

            ImGui::Separator();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (navmesh->GetFloor()) {
            ImGui::SameLine();
            if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32))) {
                navmesh->SetFloor(nullptr);
                EngineDebug::GetInstance().PrintInfo("NAVIGATIONMESH: Floor set to nullptr");
            }
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Obstacles")) {
        ImGui::Text("Obstacles: ");
        ImGui::SameLine();

        auto obstacles = navmesh->GetObstacles();

        for (size_t i = 0; i < obstacles.size(); ++i) {
            auto obs = obstacles[i];
            std::string buttonLabel = obs && obs->gameObject ? obs->gameObject->name : "None";

            ImGui::PushID(static_cast<int>(i)); // Ensure uniqueness

            ImGui::Button(buttonLabel.c_str(), ImVec2(200, 32));

            ImGui::SameLine();
            if (ImGui::Button("Select", ImVec2(75, 32))) {
                std::string popupId = "SelectObstaclePopup##" + std::to_string(i);
                ImGui::OpenPopup(popupId.c_str());
            }

            std::string popupId = "SelectObstaclePopup##" + std::to_string(i);
            if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Select a GameObject with ColliderShape:");
                ImGui::Separator();

                std::function<void(GameObject *)> drawSelectableGameObject = [&](GameObject *go) {
                    if (!go)
                        return;

                    ImGui::PushID(go->GetUUID().c_str());

                    CollisionShape *shape = go->GetComponent<CollisionShape>();
                    bool hasChildren = !go->children.empty();
                    ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (!hasChildren)
                        treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                    bool open = ImGui::TreeNodeEx(go->name.c_str(), treeFlags);

                    if (shape && ImGui::IsItemClicked()) {
                        navmesh->AddObstacle(obs);
                        EngineDebug::GetInstance().PrintInfo("NAVIGATIONMESH: Obstacle " + go->name + " added");
                        ImGui::CloseCurrentPopup();
                        ImGui::PopID();
                        return;
                    }

                    if (open && hasChildren) {
                        for (const auto &child: go->children) {
                            drawSelectableGameObject(child.get());
                        }
                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                };

                const auto root = gui.GetScene()->sceneRootObject.get();
                drawSelectableGameObject(root);

                ImGui::Separator();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            if (obs) {
                ImGui::SameLine();
                if (Panel::ColoredButton(ICON_FA_TIMES_CIRCLE_O, Colors::DarkRed, ImVec2(32, 32))) {
                    navmesh->RemoveObstacle(obs);
                    EngineDebug::GetInstance().PrintInfo("NAVIGATIONMESH: Obstacle removed");
                }
            }

            ImGui::PopID();
        }

        ImGui::PushID("AddObstacle");
        if (ImGui::Button("Add Obstacle", ImVec2(200, 32))) {
            ImGui::OpenPopup("AddObstaclePopup");
        }

        if (ImGui::BeginPopupModal("AddObstaclePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select a GameObject with ColliderShape:");
            ImGui::Separator();

            std::function<void(GameObject *)> drawSelectableGameObject = [&](GameObject *go) {
                if (!go)
                    return;

                ImGui::PushID(go->GetUUID().c_str());

                CollisionShape *shape = go->GetComponent<CollisionShape>();
                bool hasChildren = !go->children.empty();
                ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (!hasChildren)
                    treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                bool open = ImGui::TreeNodeEx(go->name.c_str(), treeFlags);

                if (shape && ImGui::IsItemClicked()) {
                    navmesh->AddObstacle(go->GetComponent<CollisionShape>());
                    EngineDebug::GetInstance().PrintInfo("NAVIGATIONMESH: Obstacle " + go->name + " added");
                    ImGui::CloseCurrentPopup();
                    ImGui::PopID();
                    return;
                }

                if (open && hasChildren) {
                    for (const auto &child: go->children) {
                        drawSelectableGameObject(child.get());
                    }
                    ImGui::TreePop();
                }

                ImGui::PopID();
            };

            const auto root = gui.GetScene()->sceneRootObject.get();
            drawSelectableGameObject(root);

            ImGui::Separator();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGui::TreePop();
    }
}