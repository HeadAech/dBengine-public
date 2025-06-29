#include "Panel.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <Helpers/Colors/Colors.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include "Singletons/Clipboard/Clipboard.h"
#include <EngineGUI/EngineGUI.h>
#include <functional>

Panel::Panel() { SetName("Panel"); 
this->uuid = UUID::generateUUID();
}
Panel::~Panel() {}

std::string Panel::GetName() { return this->name; }

void Panel::SetName(std::string name) { this->name = name; }

void Panel::ColoredText(const char *text, ImVec4 color) { 
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text(text);
    ImGui::PopStyleColor();
}

bool Panel::ColoredButton(const char *label, ImVec4 color, ImVec2 size) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, color.w));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.9f, color.y * 0.9f, color.z * 0.9f, color.w));

    bool pressed = ImGui::Button(label, size);

    ImGui::PopStyleColor(3);

    return pressed;
}

void Panel::CenterWidget(float widgetWidth) { 
    float windowWidth = ImGui::GetWindowSize().x;
    float widgetX = (windowWidth - widgetWidth) * 0.5f;

    ImGui::SetCursorPosX(widgetX);
}

bool Panel::TreeNodeExButton(const char *id, const char *label, const char *btnLabel, bool *outPressed,
                             ImGuiTreeNodeFlags baseFlags) {
    ImGuiStyle &style = ImGui::GetStyle();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
    bool open = ImGui::TreeNodeEx(id, baseFlags, "%s", label);
    ImGui::PopStyleVar(2);

    ImGui::SameLine();
    float buttonWidth = ImGui::CalcTextSize(btnLabel).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - buttonWidth);

    bool pressed = ImGui::Button(btnLabel, ImVec2(buttonWidth, 0));

    if (outPressed)
        *outPressed = pressed;

    return open;
}

bool Panel::DragVec3Widget(const std::string &label, glm::vec3 &values, float resetValue, float columnWidth) 
{
    bool changed = false;

    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushID(label.c_str());

    if (!label.empty())
        ImGui::Text("%s", label.c_str());

     if (ImGui::BeginPopupContextItem("##Vec3Context")) {
        if (ImGui::MenuItem(ICON_FA_CLIPBOARD " Copy")) {
            Clipboard::GetInstance().CopiedVec3 = values;
            ImGui::SetClipboardText(("{" + std::to_string(values.x) + ", " + std::to_string(values.y) + ", " + std::to_string(values.z) + "}").c_str());
        }
        if (ImGui::MenuItem(ICON_FA_FILES_O " Paste")) {
            values = Clipboard::GetInstance().CopiedVec3;
            changed = true;
        }
        ImGui::EndPopup();
    }

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight, lineHeight};

    EngineGUI &gui = EngineGUI::GetInstance();
    FontFamily &ff = gui.GetInterfaceFontFamily();
    // X
    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("X", Colors::DarkRed, buttonSize)) {
            values.x = resetValue;
            changed = true; 
        }
        ImGui::PopFont();

        ImGui::SameLine();
        if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
    }

    // Y
    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("Y", Colors::DarkGreen, buttonSize)) {
            values.y = resetValue;
            changed = true; 
        }
        ImGui::PopFont();


        ImGui::SameLine();
        if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
    }

    // Z
    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("Z", Colors::DarkBlue, buttonSize)) {
            values.z = resetValue;
            changed = true; 
        }
        ImGui::PopFont();


        ImGui::SameLine();
        if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f")) {
            changed = true;
        }
        ImGui::PopItemWidth();
    }

    ImGui::PopStyleVar();
    ImGui::PopID();

    return changed;
}

bool Panel::DragVec3MinMaxWidget(const std::string &label, glm::vec3 &minValues, glm::vec3 &maxValues, float resetValue,
                                 float columnWidth) 
{
    bool widget1 = DragVec3Widget(label + " Min", minValues, resetValue, columnWidth);
    bool widget2 = DragVec3Widget(label + " Max", maxValues, resetValue, columnWidth);

    if (minValues.x > maxValues.x) {
        maxValues.x = minValues.x;
    }
    if (minValues.y > maxValues.y) {
        maxValues.y = minValues.y;
    }
    if (minValues.z > maxValues.z) {
        maxValues.z = minValues.z;
    }

    if (maxValues.x < minValues.x) {
        minValues.x = maxValues.x;
    }
    if (maxValues.y < minValues.y) {
        minValues.y = maxValues.y;
    }
    if (maxValues.z < minValues.z) {
        minValues.z = maxValues.z;
    }

    return widget1 || widget2;
}

bool Panel::DragMinMaxWidget(const std::string &label, float &minValue, float &maxValue, float resetValue,
                             float columnWidth) {
    bool changed = false;

    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushID(label.c_str());

    if (!label.empty())
        ImGui::Text("%s", label.c_str());

    if (ImGui::BeginPopupContextItem("##Vec2Context")) {
        if (ImGui::MenuItem(ICON_FA_CLIPBOARD " Copy")) {
            Clipboard::GetInstance().CopiedVec2 = {minValue, maxValue};
            ImGui::SetClipboardText(("{" + std::to_string(minValue) + ", " +std::to_string(maxValue) + "}").c_str());
        }
        if (ImGui::MenuItem(ICON_FA_FILES_O " Paste")) {
            minValue = Clipboard::GetInstance().CopiedVec2.x;
            maxValue = Clipboard::GetInstance().CopiedVec2.y;
            changed = true;
        }
        ImGui::EndPopup();
    }

    ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    float lineWidth = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.x * 2.0f * 3;
    ImVec2 buttonSize = {lineWidth, lineHeight};

    EngineGUI &gui = EngineGUI::GetInstance();
    FontFamily &ff = gui.GetInterfaceFontFamily();

    // Min
    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("Min", Colors::DarkRed, buttonSize)) {
            minValue = resetValue;
            changed = true;
        }
        ImGui::PopFont();

        ImGui::SameLine();
        if (ImGui::DragFloat("##Min", &minValue, 0.1f, 0.0f, 0.0f, "%.2f")) {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
    }

    // Max
    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("Max", Colors::DarkGreen, buttonSize)) {
            maxValue = resetValue;
            changed = true;
        }
        ImGui::PopFont();


        ImGui::SameLine();
        if (ImGui::DragFloat("##Max", &maxValue, 0.1f, 0.0f, 0.0f, "%.2f")) {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
    }

    if (minValue > maxValue) {
        maxValue = minValue;
    }

    if (maxValue < minValue) {
        minValue = maxValue;
    }
   
    ImGui::PopStyleVar();
    ImGui::PopID();

    return changed;
}

bool Panel::DragVec2Widget(const std::string& label, glm::vec2& values, float resetValue, float columnWidth)
{
    bool changed = false;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushID(label.c_str());

    if (!label.empty())
        ImGui::Text("%s", label.c_str());

    if (ImGui::BeginPopupContextItem("##Vec2Context"))
    {
        if (ImGui::MenuItem(ICON_FA_CLIPBOARD " Copy"))
        {
            Clipboard::GetInstance().CopiedVec2 = values;
            ImGui::SetClipboardText(("{" + std::to_string(values.x) + ", " + std::to_string(values.y) + "}").c_str());
        }
        if (ImGui::MenuItem(ICON_FA_FILES_O " Paste"))
        {
            values = Clipboard::GetInstance().CopiedVec2;
            changed = true;
        }
        ImGui::EndPopup();
    }

    ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight, lineHeight };

    EngineGUI& gui = EngineGUI::GetInstance();
    FontFamily& ff = gui.GetInterfaceFontFamily();
    // X
    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("X", Colors::DarkRed, buttonSize))
        {
            values.x = resetValue;
            changed = true;
        }
        ImGui::PopFont();

        ImGui::SameLine();
        if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
        {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
    }

    // Y
    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("Y", Colors::DarkGreen, buttonSize))
        {
            values.y = resetValue;
            changed = true;
        }
        ImGui::PopFont();


        ImGui::SameLine();
        if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
        {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
    }

    ImGui::PopStyleVar();
    ImGui::PopID();

    return changed;
}

bool Panel::DragFloatWidget(const std::string &label, float &value, float resetValue, float columnWidth) {
    
    bool changed = false;
    ImGuiIO &io = ImGui::GetIO();
    ImGui::PushID(label.c_str());

    if (!label.empty())
        ImGui::Text("%s", label.c_str());

    if (ImGui::BeginPopupContextItem("##FloatContent")) {
        if (ImGui::MenuItem(ICON_FA_CLIPBOARD " Copy")) {
            Clipboard::GetInstance().CopiedFloat = value;
            ImGui::SetClipboardText(("{" + std::to_string(value) + "}").c_str());
        }
        if (ImGui::MenuItem(ICON_FA_FILES_O " Paste")) {
            value = Clipboard::GetInstance().CopiedFloat;
            changed = true;
        }
        ImGui::EndPopup();
    }

    ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight, lineHeight};

    EngineGUI &gui = EngineGUI::GetInstance();
    FontFamily &ff = gui.GetInterfaceFontFamily();

    {
        ImGui::PushFont(ff.GetFont(BOLD));
        if (Panel::ColoredButton("O", Colors::DarkRed, buttonSize)) {
            value = resetValue;
            changed = true;
        }
        ImGui::PopFont();

        ImGui::SameLine();
        if (ImGui::DragFloat("##O", &value, 1.f, 0.0f, 0.0f, "%.f")) {
            changed = true;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
    }

    ImGui::PopStyleVar();
    ImGui::PopID();

    return changed;
}

//bool Panel::CollapsingHeader(const char* label)
//{
//    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
//    bool* pOpen = ImGui::GetStateStorage()->GetBoolRef(GetID(label), false);
//    if (ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f)))
//    {
//        *pOpen ^= 1;
//    }
//    ImGuiStyle& style = ImGui::GetStyle();
//    ImVec2 arrow_pos = ImVec2(GetItemRectMax().x - style.FramePadding.x - GetFontSize(), GetItemRectMin().y + style.FramePadding.y);
//    ImGui::RenderArrow(GetWindowDrawList(), arrow_pos, GetColorU32(ImGuiCol_Text), *pOpen ? ImGuiDir_Down : ImGuiDir_Right);
//    ImGui::PopStyleVar();
//
//    return *pOpen;
//}

bool Panel::CollapsingHeader(const char* label, const char* buttonLabel, std::function<void()> onButtonClick)
{
    ImGui::PushID(label);

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

    // Store open state using ImGui ID
    bool* pOpen = ImGui::GetStateStorage()->GetBoolRef(ImGui::GetID(label), false);

    // Calculate full label width before drawing to set up same-line offset
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    float fullWidth = ImGui::GetContentRegionAvail().x;

    // Begin horizontal layout
    ImGui::BeginGroup();

    float reserve = onButtonClick ? 38.0f : 0.0f;

    // Draw main header button
    if (ImGui::Button(label, ImVec2(fullWidth - reserve, 0.0f))) // Reserve 40px for side button
    {
        *pOpen ^= 1;
    }
    ImGui::PopStyleVar();

    // Draw arrow manually
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 arrowPos = ImVec2(ImGui::GetItemRectMax().x - style.FramePadding.x - ImGui::GetFontSize(),
        ImGui::GetItemRectMin().y + style.FramePadding.y);
    ImGui::RenderArrow(ImGui::GetWindowDrawList(), arrowPos, ImGui::GetColorU32(ImGuiCol_Text), *pOpen ? ImGuiDir_Down : ImGuiDir_Right);

    // Draw end button on the same line
    if (onButtonClick)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::SameLine();
        if (ImGui::Button(buttonLabel, ImVec2(30.0f, 0.0f)))
        {
            onButtonClick();
        }

        ImGui::PopStyleVar();
    }
    

    ImGui::EndGroup();


    ImGui::PopID();
    return *pOpen;
}
