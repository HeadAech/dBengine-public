//
// Created by Hubert Klonowski on 15/03/2025.
//

#ifndef INPUT_H
#define INPUT_H
#include <map>
#include <glm/vec2.hpp>

#include "GLFW/glfw3.h"
#include <string>

/// <summary>
/// Class responsible for handling input events.
/// </summary>
class Input {
    

public:

    /// <summary>
    /// Boolean indicating whether the cursor is locked.
    /// </summary>
    bool m_cursorLocked = false;

    /// <summary>
    /// Boolean indicating whether the first mouse movement has occurred.
    /// </summary>
    bool m_firstMouse = true;

    Input() = default;
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    /// <summary>
    /// Sets the GLFW window for input processing.
    /// </summary>
    /// <param name="window">GLFW Window's pointer</param>
    void SetWindow(GLFWwindow *window);

    /// <summary>
    /// Returns a singleton instance of the Input class.
    /// </summary>
    /// <returns>Singleton (Input)</returns>
    static Input& GetInstance();

    /// <summary>
    /// Returns whether a key is currently pressed.
    /// </summary>
    /// <param name="key">Keycode</param>
    /// <returns>Boolean</returns>
    bool IsKeyPressed(int key);

    /// <summary>
    /// Returns whether an action is currently pressed.
    /// </summary>
    /// <param name="action">Name of the action (string)</param>
    /// <returns>Boolean</returns>
    bool IsActionPressed(std::string action);

    /// <summary>
    /// Returns whether a key is currently released.
    /// </summary>
    /// <param name="key">Keycode</param>
    /// <returns>Boolean</returns>
    bool IsKeyReleased(int key);

    /// <summary>
    /// Returns whether an action is currently released.
    /// </summary>
    /// <param name="action">Name of the action (string)</param>
    /// <returns>Boolean</returns>
    bool IsActionReleased(std::string action);

    /// <summary>
    /// Returns whether a key was just pressed.
    /// </summary>
    /// <param name="key">Keycode</param>
    /// <returns>Boolean</returns>
    bool IsKeyJustPressed(int key);

    /// <summary>
    /// Returns whether an action was just pressed.
    /// </summary>
    /// <param name="action">Name of the action (string)</param>
    /// <returns>Boolean</returns>
    bool IsActionJustPressed(std::string action);

    /// <summary>
    /// Returns whether a key was just released.
    /// </summary>
    /// <param name="key">Keycode</param>
    /// <returns>Boolean</returns>
    bool IsKeyJustReleased(int key);

    /// <summary>
    /// Returns whether an action was just released.
    /// </summary>
    /// <param name="action">Name of the action (string)</param>
    /// <returns>Boolean</returns>
    bool IsActionJustReleased(std::string action);

    /// <summary>
    /// Processes input events for the given GLFW window.
    /// </summary>
    /// <param name="window">GLFW window's pointer</param>
    void ProcessInput(GLFWwindow* window);

    /// <summary>
    /// Maps a key to an action name.
    /// </summary>
    /// <param name="actionName">Action's name (string)</param>
    /// <param name="key">Keycode (int)</param>
    /// <param name="isMouseButton">Is that action a mouse button (bool)</param>
    void RegisterAction(std::string actionName, int key, bool isMouseButton = false);

    /// <summary>
    /// Sets the cursor lock state for the GLFW window.
    /// </summary>
    /// <param name="isLocked">Lock state (bool)</param>
    void SetCursorLocked(bool isLocked);

    /// <summary>
    /// Processes mouse movement events for the given GLFW window.
    /// </summary>
    /// <param name="window">GLFW window's pointer</param>
    /// <param name="xpos"></param>
    /// <param name="ypos"></param>
    void ProcessMouseMovement(GLFWwindow* window, double xpos, double ypos);

    /// <summary>
    /// Gets the mouse offset since the last frame in the X direction.
    /// </summary>
    /// <returns>Offset (double)</returns>
    double GetMouseOffsetX();

    /// <summary>
    /// Gets the mouse offset since the last frame in the Y direction.
    /// </summary>
    /// <returns>Offset (double)</returns>
    double GetMouseOffsetY();

    /// <summary>
    /// Gets the current mouse position in the X direction.
    /// </summary>
    /// <returns>Mouse position in X axis (double)</returns>
    double GetMouseX();

    /// <summary>
    /// Gets the current mouse position in the Y direction.
    /// </summary>
    /// <returns>Mouse position in Y axis (double)</returns>
    double GetMouseY();

    /// <summary>
    /// Closes the GLFW window and quits the application.
    /// </summary>
    void Quit();

    /// <summary>
    /// Returns the current mouse position.
    /// </summary>
    /// <returns>Vector (vec2)</returns>
    glm::vec2 GetMousePosition();

    /// <summary>
    /// Returns a reference to a map of all registered actions with their keycodes.
    /// </summary>
    /// <returns>Map (string, int)</returns>
    const std::map<std::string, int> &GetActions();

    /// <summary>
    /// Returns a reference to a map of all registered mouse actions with their keycodes.
    /// </summary>
    /// <returns>Map (string, int)</returns>
    const std::map<std::string, int> &GetMouseActions();

    private:

    std::map<int, bool> currentKeys;
    std::map<int, bool> previousKeys;
    std::map<std::string, int> actions;
    std::map<std::string, int> mouseActions;

    bool isKeyMouseButton(int key);

    double lastMouseX = 0;
    double lastMouseY = 0;
    double mouseOffsetX = 0;
    double mouseOffsetY = 0;

    GLFWwindow* window;
};



#endif //INPUT_H
