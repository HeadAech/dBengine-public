#ifndef PANEL_H
#define PANEL_H
#define IMGUI_DEFINE_MATH_OPERATORS
#include <string>
#include <imgui.h>
#include <Helpers/UUID/UUID.h>
#include <glm/glm.hpp>
#include <functional>

/// <summary>
/// Utility class for creating and managing views in the engine's editor's interface.
/// </summary>
class Panel {

    std::string name;

public:
    Panel();
    ~Panel();
    
    std::string uuid;

    /// <summary>
    /// Virtual function to render the panel.
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// Returns the name of the panel.
    /// </summary>
    /// <returns>String</returns>
    std::string GetName();

    /// <summary>
    /// Sets the name of the panel.
    /// </summary>
    /// <param name="name">Name of the panel</param>
    void SetName(std::string name);

    /// <summary>
    /// Draws a colored text widget.
    /// </summary>
    /// <param name="text">Text to display</param>
    /// <param name="color">Color (ImVec2)</param>
    static void ColoredText(const char *text, ImVec4 color);

    /// <summary>
    /// Draws a colored button with the specified label, color, and size.
    /// </summary>
    /// <param name="label">Text inside the button</param>
    /// <param name="color">Color of the background (ImVec2)</param>
    /// <param name="size">Size of the buttom (ImVec2)</param>
    /// <returns></returns>
    static bool ColoredButton(const char *label, ImVec4 color, ImVec2 size = ImVec2(0, 0));

    /// <summary>
    /// Helper function to center a widget in the current window.
    /// </summary>
    /// <param name="widgetWidth">Width of the widget to center</param>
    static void CenterWidget(float widgetWidth);

    /// <summary>
    /// Draws a tree node with a button next to it.
    /// <para>
    /// CURRENTLY NOT WORKING PROPERLY.
    /// </para>
    /// </summary>
    /// <param name="id"></param>
    /// <param name="label"></param>
    /// <param name="btnLabel"></param>
    /// <param name="outPressed"></param>
    /// <param name="baseFlags"></param>
    /// <returns></returns>
    static bool TreeNodeExButton(const char *id, const char *label, const char *btnLabel, bool *outPressed = nullptr,
                                 ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Framed |
                                                                ImGuiTreeNodeFlags_SpanAvailWidth);

    /// <summary>
    /// Draws a widget for editing a vector 3 value.
    /// </summary>
    /// <param name="label">Text to display above the widget</param>
    /// <param name="values">Vec3 values</param>
    /// <param name="resetValue">Value to reset according values when pressed on the buttons</param>
    /// <param name="columnWidth">Width of the column</param>
    /// <returns>Changed state (bool)</returns>
    static bool DragVec3Widget(const std::string &label, glm::vec3 &values, float resetValue = 0.0f,
                               float columnWidth = 100.0f);

    /// <summary>
    /// Draws a widget that automatically adjusts the min and max values of a vector 3.
    /// </summary>
    /// <param name="label">Text to display</param>
    /// <param name="minValues">Min values (vec3)</param>
    /// <param name="maxValues">Max values (vec3)</param>
    /// <param name="resetValue">Value to reset according values when pressed on the buttons</param>
    /// <param name="columnWidth">Width of the columns</param>
    /// <returns>Changed state (bool)</returns>
    static bool DragVec3MinMaxWidget(const std::string &label, glm::vec3 &minValues, glm::vec3& maxValues, float resetValue = 0.0f,
                               float columnWidth = 100.0f);


    /// <summary>
    /// Draws a widget that automatically adjusts the min and max values of a float.
    /// </summary>
    /// <param name="label">Text to display</param>
    /// <param name="minValue">Min value (float)</param>
    /// <param name="maxValue">Max value(float)</param>
    /// <param name="resetValue">Value to reset according values when pressed on the buttons</param>
    /// <param name="columnWidth">Width of the columns</param>
    /// <returns>Changed state (bool)</returns>
    static bool DragMinMaxWidget(const std::string &label, float &minValue, float &maxValue, float resetValue = 0.0f,
                                 float columnWidth = 100.0f);

    /// <summary>
    /// Draws a widget for editing a vector 2 value.
    /// </summary>
    /// <param name="label">Text to display above the widget</param>
    /// <param name="values">Vec2 values</param>
    /// <param name="resetValue">Value to reset according values when pressed on the buttons</param>
    /// <param name="columnWidth">Width of the columns</param>
    /// <returns>Changed state (bool)</returns>
    static bool DragVec2Widget(const std::string &label, glm::vec2 &values, float resetValue = 0.0f,
                               float columnWidth = 100.0f);

    /// <summary>
    /// Draws a widget for editing a float value.
    /// </summary>
    /// <param name="label">Text to display above the widget</param>
    /// <param name="value">float value</param>
    /// <param name="resetValue">Value to reset according values when pressed on the buttons</param>
    /// <param name="columnWidth">Width of the columns</param>
    /// <returns>Changed state (bool)</returns>
    static bool DragFloatWidget(const std::string &label, float &value, float resetValue = 0.0f,
                               float columnWidth = 100.0f);


    static bool CollapsingHeader(const char* label, const char* buttonLabel = NULL, std::function<void()> onButtonClick = NULL);
};


#endif // !PANEL_H

