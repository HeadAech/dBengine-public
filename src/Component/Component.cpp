//
// Created by Hubert Klonowski on 14/03/2025.
//

#include "Component.h"

#include <iostream>

#include "GameObject/GameObject.h"

Component::Component() { this->uuid = UUID::generateUUID(); };


void Component::Update(float deltaTime) {}

void Component::Render() {}

void Component::Disable() {
    enabled = false;
}

void Component::Enable() {
    enabled = true;
}

void Component::Die() {
    std::cout << "Killing Component of gameobject " << gameObject->name << std::endl;
    this->~Component();
}

std::string Component::GetUUID() { return this->uuid; }
