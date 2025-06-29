#pragma once

#ifndef COLORS_H
#define COLORS_H

#include <imgui.h>

/// <summary>
/// Utility namespace for defining colors using ImGui's ImColor.
/// </summary>
namespace Colors {
    inline const ImVec4 Red = ImColor(255, 0, 0, 255);
    inline const ImVec4 Green = ImColor(0, 255, 0, 255);
    inline const ImVec4 Blue = ImColor(0, 0, 255, 255);
    inline const ImVec4 Transparent = ImColor(0, 0, 0, 0);
    inline const ImVec4 LightRed = ImColor(236, 112, 99, 255);
    inline const ImVec4 DarkRed = ImColor(176, 58, 46, 255);
    inline const ImVec4 Purple = ImColor(125, 60, 152, 255);
    inline const ImVec4 Yellow = ImColor(251, 196, 15, 255);
    inline const ImVec4 Orange = ImColor(211, 84, 0, 255);
    inline const ImVec4 LightGreen = ImColor(46, 204, 113, 255);
    inline const ImVec4 DarkBlue = ImColor(26, 82, 118, 255);
    inline const ImVec4 DarkGreen = ImColor(25, 111, 61, 255);
    inline const ImVec4 Cyan = ImColor(0, 255, 255, 255);
    inline const ImVec4 DarkGray = ImColor(66, 73, 73, 255);
    inline const ImVec4 Gray = ImColor(112, 123, 124, 255);
    inline const ImVec4 White = ImColor(255, 255, 255, 255);

}


#endif // !COLORS_H
