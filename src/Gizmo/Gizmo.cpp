#include "Gizmo.h"
#include <algorithm>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <vector>

#include "InputManager/Input.h"
#include "dBengine/EngineDebug/EngineDebug.h"
#include <imgui_impl/imgui_impl_opengl3_loader.h>
#include <imguizmo/ImGuizmo.h>

float camDistance = 8.f;
static ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
static bool useSnap = false;
static float snap[3] = {1.f, 1.f, 1.f};
static const glm::mat4 identityMatrix = glm::mat4(1.0f);
static bool snapToggled = false;


ImGuizmo::OPERATION Gizmo::GetCurrentOperation() { return mCurrentGizmoOperation; }
ImGuizmo::MODE Gizmo::GetCurrentMode() { return mCurrentGizmoMode; }
bool Gizmo::GetUseSnap() { return useSnap; }
bool Gizmo::GetSnapToggled() { return snapToggled; }

void Gizmo::TransformStart(glm::mat4 &cameraView, glm::mat4 &cameraProjection, glm::mat4 &matrix) {
    static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
    static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
    static bool boundSizing = false;
    static bool boundSizingSnap = false;

    if (!Input::GetInstance().m_cursorLocked) {

        if (Input::GetInstance().IsActionJustPressed("gizmo_translate") && !ImGui::IsMouseDragging(0)) {
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        }
        if (Input::GetInstance().IsActionJustPressed("gizmo_rotate") && !ImGui::IsMouseDragging(0)) {
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        }
        if (Input::GetInstance().IsActionJustPressed("gizmo_scale") && !ImGui::IsMouseDragging(0)) {
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        }

        if (!GetSnapToggled()) {
            useSnap = Input::GetInstance().IsActionPressed("gizmo_snapping");
        } else {
            useSnap = true;
        }
    }
    ImVec2 windowPos = ImGui::GetWindowPos();
    
    ImVec2 windowSize = ImGui::GetWindowSize();

    ImGuiIO &io = ImGui::GetIO();



    // Set ImGuizmo to draw within the viewport window
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
}

void Gizmo::TransformEnd() {
    // No window to end, so this is empty
}

/// <summary>
/// Render and manage a Edit gizmo for given object.
/// </summary>
/// <param name="cameraView"></param>
/// <param name="cameraProjection"></param>
/// <param name="gameObject">Selected game object to edit its transform</param>
void Gizmo::EditTransform(glm::mat4 &cameraView, glm::mat4 &cameraProjection, GameObject* gameObject) {
    ImGuiIO &io = ImGui::GetIO();
    float windowWidth = (float) ImGui::GetWindowWidth();
    float windowHeight = (float) ImGui::GetWindowHeight();

    ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);

    glm::mat4 tempMatrix = gameObject->transform.GetModelMatrix();

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), mCurrentGizmoOperation,
                         mCurrentGizmoMode, glm::value_ptr(tempMatrix), NULL, useSnap ? &snap[0] : NULL);

    // Only apply changes if ImGuizmo is used
    if (ImGuizmo::IsUsing()) {

        glm::mat4 newMatrix;
        if (gameObject->parent) {
            glm::mat4 parentWorldMatrix = gameObject->parent->transform.GetModelMatrix();
            newMatrix = glm::inverse(parentWorldMatrix) * tempMatrix;
        } else {
            newMatrix = tempMatrix; // root nodes [orphans], local = world 
        }

        glm::vec3 newPosition, newRotation, newScale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(newMatrix), &newPosition[0], &newRotation[0],
                                              &newScale[0]);

        if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) {
            gameObject->transform.SetLocalPosition(newPosition);
        } else if (mCurrentGizmoOperation == ImGuizmo::ROTATE) {
            Camera *cam = gameObject->GetComponent<Camera>();
            if (cam) {
                glm::vec3 rotationVec = newRotation;
                glm::vec3 oldRotationVec = gameObject->transform.GetEulerRotation();

                glm::vec3 delta = rotationVec - oldRotationVec;
                cam->Pitch += delta.x;
                cam->Yaw += delta.y;
                cam->UpdateCameraVectors();
            } else {
                gameObject->transform.SetEulerRotation(newRotation);
            }
            
        } else if (mCurrentGizmoOperation == ImGuizmo::SCALE) {
            gameObject->transform.SetScale(newScale);
        }

        gameObject->transform.m_isDirty = true;
    }
}

void Gizmo::SetGizmoOperation(ImGuizmo::OPERATION operation) {
    mCurrentGizmoOperation = operation;
}

void Gizmo::SetGizmoMode(ImGuizmo::MODE mode) {
    mCurrentGizmoMode = mode;
}

void Gizmo::SetSnapping(bool mode) {
    useSnap = mode;

}

void Gizmo::SetSnapToggle(bool mode) {
    snapToggled = mode; }


/// <summary>
/// render and manage the viewmanipulate cube placed in editor viewport
/// </summary>
/// <param name="cameraView"></param>
void Gizmo::DrawViewManipulate(Camera* cam, float length) {
    
    glm::mat4 cameraView = cam->GetViewMatrix();
    ImGuiIO &io = ImGui::GetIO();
    float viewManipulateRight = io.DisplaySize.x;
    float viewManipulateTop = 0;
    static ImGuiWindowFlags gizmoWindowFlags = 0;
  
   
    ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
    ImGuizmo::SetDrawlist();
    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
    viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
    viewManipulateTop = ImGui::GetWindowPos().y;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0;
   
    ImGuizmo::ViewManipulate(glm::value_ptr(cameraView), length, ImVec2(viewManipulateRight - 128, viewManipulateTop),
                             ImVec2(128, 128), 0x10101010, cam);

    ImGui::PopStyleColor(1);
}