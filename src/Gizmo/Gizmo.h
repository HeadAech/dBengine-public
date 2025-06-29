#ifndef GIZMO_H
#define GIZMO_H

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <imguizmo/ImGuizmo.h>
#include <Components/Transform/Transform.h>
#include <Components/Camera/Camera.h>

class Gizmo {
public:
    static ImGuizmo::OPERATION GetCurrentOperation();
    static ImGuizmo::MODE GetCurrentMode();
    static bool GetUseSnap();
    static bool GetSnapToggled();

    static void TransformStart(glm::mat4 &cameraView, glm::mat4 &cameraProjection, glm::mat4 &matrix);
    static void TransformEnd();
    static void EditTransform(glm::mat4 &cameraView, glm::mat4 &cameraProjection, GameObject* gameObject);

    static void SetGizmoOperation(ImGuizmo::OPERATION operation);
    static void SetGizmoMode(ImGuizmo::MODE mode);
    static void SetSnapping(bool mode);
    static void SetSnapToggle(bool mode);

    static void DrawViewManipulate(Camera* cam, float length);
};

#endif
