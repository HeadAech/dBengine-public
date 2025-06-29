//
// Created by Hubert Klonowski on 15/03/2025.
//

#include "Input.h"

#include <iostream>
#include <ostream>
#include <algorithm>

#include "Signal/Signals.h"


Input &Input::GetInstance() {
    static Input instance;
    return instance;
}


bool Input::IsKeyPressed(int key) {
    return currentKeys[key];
}

bool Input::IsActionPressed(std::string action) {
    int key = actions[action];
    return IsKeyPressed(key);
}


bool Input::IsKeyJustPressed(int key) {
    return currentKeys[key] && !previousKeys[key];
}

bool Input::IsActionJustPressed(std::string action) {
    int key = actions[action];
    return IsKeyJustPressed(key);
}

bool Input::IsKeyJustReleased(int key) {
    return !currentKeys[key] && previousKeys[key];
}

bool Input::IsActionJustReleased(std::string action) {
    int key = actions[action];
    return IsKeyJustReleased(key);
}

bool Input::IsKeyReleased(int key) {
    return !currentKeys[key];
}

bool Input::IsActionReleased(std::string action) {
    int key = actions[action];
    return IsKeyReleased(key);
}

void Input::ProcessInput(GLFWwindow *window) {
    for (auto& [key, state] : currentKeys) {
        // Update the previous state
        previousKeys[key] = state;
        // Get the current state from GLFW
        if (isKeyMouseButton(key))
            currentKeys[key] = glfwGetMouseButton(window, key) == GLFW_PRESS;
        else
            currentKeys[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }
}

void Input::RegisterAction(std::string actionName, int key, bool isMouseButton) {
    currentKeys[key] = false;
    previousKeys[key] = false;
    actions[actionName] = key;
    if (isMouseButton)
        mouseActions[actionName] = key;
}

bool Input::isKeyMouseButton(int key) {
    bool inActions = std::any_of(actions.begin(), actions.end(), [key](const auto& pair) { return pair.second == key; });
    bool inMouseActions = std::any_of(mouseActions.begin(), mouseActions.end(), [key](const auto& pair) { return pair.second == key; });

    return inActions && inMouseActions;
}

void Input::SetCursorLocked(bool isLocked) {
    m_cursorLocked = isLocked;
    if (isLocked) {
        m_firstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Input::ProcessMouseMovement(GLFWwindow *window, double xpos, double ypos) {

        if (m_firstMouse)
        {
            lastMouseX = xpos;
            lastMouseY = ypos;
            m_firstMouse = false;
        }

        mouseOffsetX = xpos - lastMouseX;
        mouseOffsetY = lastMouseY - ypos; // reversed since y-coordinates go from bottom to top

        lastMouseX = xpos;
        lastMouseY = ypos;
        Signals::CursorOffsetChanged.emit(mouseOffsetX, mouseOffsetY);


}

double Input::GetMouseOffsetX() {
    return mouseOffsetX;
}
double Input::GetMouseOffsetY() {
    return mouseOffsetY;
}

double Input::GetMouseX() {
    return lastMouseX;
}

double Input::GetMouseY() {
    return lastMouseY;
}

glm::vec2 Input::GetMousePosition() {
    return glm::vec2(GetMouseX(), GetMouseY()); }

const std::map<std::string, int> &Input::GetActions() { return actions; }

const std::map<std::string, int> &Input::GetMouseActions() { return mouseActions; }

void Input::SetWindow(GLFWwindow *window) {
    this->window = window;
}

void Input::Quit() {
    glfwSetWindowShouldClose(window, true);
}
