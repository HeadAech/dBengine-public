//
// Created by Hubert Klonowski on 16/03/2025.
//

#include "Lua_GameObject.h"

#include "Components/LuaComponent/LuaComponent.h"
#include "GameObject/GameObject.h"


Lua_GameObject::Lua_GameObject(LuaComponent *lua_component) : Lua(lua_component) { 
    this->uuid = UUID::generateUUID();
    Register(); 
}

void Lua_GameObject::Register() {
    lua_component->L.new_usertype<Lua_GameObject>(
            "GameObject", "Test", &Lua_GameObject::Test, "GetEulerRotation", &Lua_GameObject::GetEulerRotation,
            "SetEulerRotation", &Lua_GameObject::SetEulerRotation, "GetLocalPosition",
            &Lua_GameObject::GetLocalPosition, "SetLocalPosition", &Lua_GameObject::SetLocalPosition, "GetScale",
            &Lua_GameObject::GetScale, "SetScale", &Lua_GameObject::SetScale, "GetQuatRotation",
            &Lua_GameObject::GetQuatRotation, "SetQuatRotation", &Lua_GameObject::SetQuatRotation, "MultiplyQuats",
            &Lua_GameObject::MultiplyQuatsTables, "Slerp", &Lua_GameObject::Slerp, "GetUUID",
            &Lua_GameObject::GetUUID );
    lua_component->L["GameObject"] = this;
}

void Lua_GameObject::Test() {
    sol::table table = lua_component->L.create_table();
    std::cout << "Lua_GameObject::Test()" << std::endl;
}

std::string Lua_GameObject::GetUUID() {
    return lua_component->gameObject->GetUUID();
}

sol::table Lua_GameObject::GetEulerRotation() {
    glm::vec3 rotation = lua_component->gameObject->transform.GetEulerRotation();
    try {
        sol::table rotation_table = lua_component->L.create_table();
        rotation_table["x"] = rotation.x;
        rotation_table["y"] = rotation.y;
        rotation_table["z"] = rotation.z;
        return rotation_table;
    } catch (const std::exception &e) {
        std::cerr << "Error creating table: " << e.what() << std::endl;
        return sol::table(); // Return an empty table or handle error
    }
}

void Lua_GameObject::SetEulerRotation(float x, float y, float z) {
    lua_component->gameObject->transform.SetEulerRotation({x, y, z});
}

sol::table Lua_GameObject::GetScale() {
    glm::vec3 scale = lua_component->gameObject->transform.GetScale();
    sol::table table = lua_component->L.create_table();
    table["x"] = scale.x;
    table["y"] = scale.y;
    table["z"] = scale.z;
    return table;
}

sol::table Lua_GameObject::GetLocalPosition() {
    glm::vec3 local_position = lua_component->gameObject->transform.GetLocalPosition();
    sol::table table = lua_component->L.create_table();
    table["x"] = local_position.x;
    table["y"] = local_position.y;
    table["z"] = local_position.z;
    return table;
}

void Lua_GameObject::SetLocalPosition(float x, float y, float z) {
    lua_component->gameObject->transform.SetLocalPosition({x, y, z});
}

void Lua_GameObject::SetScale(float x, float y, float z) { 
    lua_component->gameObject->transform.SetScale({x, y, z});
}

sol::table Lua_GameObject::GetQuatRotation() {
    glm::quat rotation = lua_component->gameObject->transform.GetQuatRotation();
    try {
        sol::table rotation_table = lua_component->L.create_table();
        rotation_table["w"] = rotation.w;
        rotation_table["x"] = rotation.x;
        rotation_table["y"] = rotation.y;
        rotation_table["z"] = rotation.z;
        return rotation_table;
    } catch (const std::exception &e) {
        std::cerr << "Error creating quaternion table: " << e.what() << std::endl;
        return sol::table(); // Return an empty table on error
    }
}


void Lua_GameObject::SetQuatRotation(sol::table q1) {
    lua_component->gameObject->transform.SetQuatRotation(glm::quat(q1["w"], q1["x"], q1["y"], q1["z"]));
}

sol::table Lua_GameObject::MultiplyQuatsTables(sol::table q1, sol::table q2) {
    glm::quat result = glm::quat(q1["w"], q1["x"], q1["y"], q1["z"]) * glm::quat(q2["w"], q2["x"], q2["y"], q2["z"]);
    try {
        sol::table quat_table = lua_component->L.create_table();
        quat_table["w"] = result.w;
        quat_table["x"] = result.x;
        quat_table["y"] = result.y;
        quat_table["z"] = result.z;
        return quat_table;
    } catch (const std::exception &e) {
        std::cerr << "Error creating quaternion table: " << e.what() << std::endl;
        return sol::table(); // Return an empty table on error
    }
}

sol::table Lua_GameObject::Slerp(sol::table q1, sol::table q2, float weight) {
    glm::quat result = glm::mix(glm::quat(q1["w"], q1["x"], q1["y"], q1["z"]),
                                glm::quat(q2["w"], q2["x"], q2["y"], q2["z"]), weight);
    try {
        sol::table quat_table = lua_component->L.create_table();
        quat_table["w"] = result.w;
        quat_table["x"] = result.x;
        quat_table["y"] = result.y;
        quat_table["z"] = result.z;
        return quat_table;
    } catch (const std::exception &e) {
        std::cerr << "Error creating quaternion table: " << e.what() << std::endl;
        return sol::table(); // Return an empty table on error
    }
}
