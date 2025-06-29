#include "Control.h"
#include <Shader/Shader.h>
#include <Singletons/Ref/Ref.h>

using namespace UI;

Control::Control() {
    name = "Control"; 
    icon = ICON_FA_WINDOW_MAXIMIZE;
}

Control::~Control() {}

void Control::SetShader(std::shared_ptr<Shader> pShader) { p_Shader = pShader; }

std::shared_ptr<Shader> Control::GetShader() const { return p_Shader; }

glm::vec2 Control::GetAnchoredPosition() const
{
    glm::vec2 anchorPoint = Position;
    glm::vec2 screenRes = Ref::ScreenResolution;

    switch (anchor)
    {
        case Anchor::TopLeft:
            anchorPoint.y = screenRes.y - Position.y - Size.y;
            break;
        case Anchor::TopCenter:
            anchorPoint.x = (screenRes.x - Size.x) / 2.0f + Position.x;
            anchorPoint.y = screenRes.y - Position.y - Size.y;
            break;
        case Anchor::TopRight:
            anchorPoint.x = screenRes.x - Position.x - Size.x;
            anchorPoint.y = screenRes.y - Position.y - Size.y;
            break;
        case Anchor::MiddleLeft:
            anchorPoint.y = (screenRes.y - Size.y) / 2.0f + Position.y;
            break;
        case Anchor::MiddleCenter:
            anchorPoint.x = (screenRes.x - Size.x) / 2.0f + Position.x;
            anchorPoint.y = (screenRes.y - Size.y) / 2.0f + Position.y;
            break;
        case Anchor::MiddleRight:
            anchorPoint.x = screenRes.x - Position.x - Size.x;
            anchorPoint.y = (screenRes.y - Size.y) / 2.0f + Position.y;
            break;
        case Anchor::BottomLeft:
            anchorPoint.y = Position.y; // Offset upward from bottom
            break;
        case Anchor::BottomCenter:
            anchorPoint.x = (screenRes.x - Size.x) / 2.0f + Position.x;
            anchorPoint.y = Position.y; // Offset upward from bottom
            break;
        case Anchor::BottomRight:
            anchorPoint.x = screenRes.x - Position.x - Size.x;
            anchorPoint.y = Position.y; // Offset upward from bottom
            break;
    }

    return anchorPoint;
}