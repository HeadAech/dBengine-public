#include "DebugPanel.h"
#include <EngineGUI/EngineGUI.h>
#include <dBengine/EngineDebug/EngineDebug.h>
#include <dBengine/EngineSettings/EngineSettings.h>
#include <dBphysics/dBphysics.h>
#include <imgui_internal.h>
#include <Components/CollisionShape/CollisionShape.h>
#include <dBrender/PostProcessing/Shake/CameraShake.h>

DebugPanel::DebugPanel() { SetName("Debug"); }

void DebugPanel::Draw() {
    EngineGUI &gui = EngineGUI::GetInstance();
    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 200), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Debug"), ImGuiWindowFlags_NoTitleBar) {
        EngineDebug &engineDebug = EngineDebug::GetInstance();

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::PushStyleColor(ImGuiCol_PlotLines, {255, 0, 0, 255});
        ImGui::PlotLines("Frame times", gui.frameTimes.data(), gui.frameTimes.size(), gui.frameTimes.size(), 0, 0, 0.15,
                         ImVec2(0, 50));
        ImGui::PopStyleColor();
        ImGui::Separator();

        ImGui::Text("dBrender - Rendering Statistics");
        ImGui::Text("Draw Calls: %d", engineDebug.drawCalls);
        ImGui::Text("Instances Count: %d", engineDebug.instancesCount);
        ImGui::Text("Update Time: %.3f ms", engineDebug.updateTime);
        // ImGui::Text("Input Latency: %.3f ms", engineDebug.inputLatency);

        if (Panel::CollapsingHeader("PostProcessing"))
        {
            
            if (ImGui::TreeNode("Camera Shake"))
            {
                auto& cameraShake = CameraShake::GetInstance();
                static float cameraShakeIntensity = 0.1f;
                static float cameraShakeDuration = 0.4f;

                ImGui::DragFloat("Camera Shake Intensity", &cameraShakeIntensity, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Camera Shake Duration", &cameraShakeDuration, 0.01f, 0.0f, 5.0f);
                if (ImGui::Button("Trigger Camera Shake"))
                {
                    cameraShake.TriggerShake(cameraShakeIntensity, cameraShakeDuration);
                }

                ImGui::TreePop();
            }
            
            auto& settings = EngineSettings::GetEngineSettings();

            if (ImGui::TreeNode("Glitch Effect"))
            {
                ImGui::Text("Enabled ");
                ImGui::SameLine();
                ImGui::Checkbox("##EnableGlitchEffect", &settings.enableGlitchEffect);

                ImGui::Text("Intensity: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##GlitchEffectIntensity", &settings.glitchEffectIntensity, 0.01f, 0.0f, 1.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Frequency: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##GlitchEffectFrequency", &settings.glitchEffectFrequency, 0.1f, 0.0f, 100.0f);
                ImGui::PopItemWidth();

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Vignette"))
            {
                ImGui::Text("Strength: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##VignetteIntensity", &settings.vignetteStrength, 0.01f, 0.0f, 1.0f);
                ImGui::PopItemWidth();

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("HUD Effects"))
            {
                ImGui::Text("Enabled distortion: ");
                ImGui::SameLine();
                ImGui::Checkbox("##DistortionEnabled", &settings.distortionEnabled);

                ImGui::Text("Fisheye Strength: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##FisheyeStrength", &settings.fisheyeStrength, 0.01f, 0.0f, 1.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Chromatic Strength: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##ChromaticStrength", &settings.chromaticStrength, 0.001f, 0.0f, 1.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Shadow Color: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::ColorEdit3("##ShadowColor", (float*) &settings.shadowColor);
                ImGui::PopItemWidth();

                ImGui::Text("Shadow Offset: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                Panel::DragVec2Widget("##ShadowHUDOffset", settings.shadowOffset, 0);
                ImGui::PopItemWidth();

                ImGui::Text("Shadow Radius: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##ShadowRadius", &settings.shadowRadius, 0.0001f, 0.0f, 1.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Shadow Intensity: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##ShadowIntensity", &settings.shadowIntensity, 0.01f, 0.0f, 1.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Displacement Strength: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##DisplacementStrength", &settings.displacementStrength, 0.001f, 0.0f, 1.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Displacement Speed: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##DisplacementSpeed", &settings.displacementSpeed, 0.1f, 0.0f, 100.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Scanline Height: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##ScanlineHeight", &settings.scanlineHeight, 0.1f, 0.0f, 50.0f);
                ImGui::PopItemWidth();

                ImGui::Text("Scanline Probability: ");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::DragFloat("##ScanlineProbability", &settings.scanlineProbability, 0.001f, 0.0f, 4.0f);
                ImGui::PopItemWidth();

                ImGui::TreePop();
            }

            if (ImGui::Button("Apply Properties"))
            {
                Signals::PostProcessing_ApplyProperties.emit();
            }
        }

        

        ImGui::Separator();

        ImGui::Text("Debug depth map");
        ImGui::SameLine();
        ImGui::Checkbox("##DrawDepthMapDir", &EngineSettings::GetEngineSettings().m_showDirectionalLightDepthMap);

        ImGui::Text("Enable glitch shader");
        ImGui::SameLine();
        ImGui::Checkbox("##EnableGlitchShader", &EngineSettings::GetEngineSettings().enableGlitchShader);

        /*ImGui::Text("Debug Navigation Mesh");
        ImGui::SameLine();
        ImGui::Checkbox("##DrawNavMesh", &EngineSettings::GetEngineSettings().renderNavigationMesh);

        ImGui::Text("Debug Agent Paths");
        ImGui::SameLine();
        ImGui::Checkbox("##DrawAgentPaths", &EngineSettings::GetEngineSettings().renderAgentPaths);*/


        if (ImGui::CollapsingHeader("Octree Debug")) {
            auto &physics = dBphysics::GetInstance();

            bool octreeVisible = physics.IsOctreeVisible();
            if (ImGui::Checkbox("Show Octree Wireframe", &octreeVisible)) {
                physics.SetOctreeVisible(octreeVisible);
            }

            auto debugInfo = physics.GetOctreeDebugInfo();

            ImGui::Text("Octree Nodes: %d", debugInfo.size());

            if (ImGui::BeginTable("OctreeStats", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Node ID");
                ImGui::TableSetupColumn("Depth");
                ImGui::TableSetupColumn("Colliders");
                ImGui::TableHeadersRow();

                for (const auto &info: debugInfo) {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", info.nodeId);

                    ImGui::TableNextColumn();
                    if (info.depth == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    } else if (info.depth == 1) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    }
                    ImGui::Text("%d", info.depth);
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", info.colliderCount);
                }

                ImGui::EndTable();
            }

            int totalColliders = 0;
            std::unordered_map<int, int> collidersPerDepth;

            for (const auto &info: debugInfo) {
                totalColliders += info.colliderCount;
                collidersPerDepth[info.depth] += info.colliderCount;
            }

            ImGui::Text("Total Colliders: %d", totalColliders);

            if (ImGui::BeginTable("ColliderDistribution", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Depth");
                ImGui::TableSetupColumn("Colliders");
                ImGui::TableHeadersRow();

                for (int depth = 0; depth <= 2; depth++) {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();

                    if (depth == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        ImGui::Text("Level %d (Root)", depth);
                    } else if (depth == 1) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
                        ImGui::Text("Level %d", depth);
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                        ImGui::Text("Level %d", depth);
                    }
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", collidersPerDepth[depth]);
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();
    }
}
