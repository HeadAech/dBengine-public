#pragma once
#ifndef SERIALIZETYPES_H
#define SERIALIZETYPES_H

#include <type_traits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <vector>
#include <map>
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include <sol.hpp>

#include <Helpers/TimerHelper/TimerHelper.h>

#include <Scene/scene.h>
#include <GameObject/GameObject.h>
#include "Component/Component.h"
#include "Components/AudioListener/AudioListener.h"
#include "Components/AudioSource/AudioSource.h"
#include "Components/Camera/Camera.h"
#include "Components/CollisionShape/CollisionShape.h"

#include "Components/Lights/Light.h"
#include "Components/Lights/LocalLight.h"
#include "Components/Lights/DirectionalLight/DirectionalLight.h"
#include "Components/Lights/PointLight/PointLight.h"
#include "Components/Lights/SpotLight/SpotLight.h"

#include "Components/LuaComponent/Lua.h"
#include "Components/LuaComponent/LuaComponent.h"
#include "Components/LuaComponent/Lua_Camera/Lua_Camera.h"
#include "Components/LuaComponent/Lua_GameObject/Lua_GameObject.h"
#include "Components/LuaComponent/Lua_Input/Lua_Input.h"
#include "Components/LuaComponent/Lua_TextRenderer/Lua_TextRenderer.h"

#include "Components/MeshInstance/MeshInstance.h"
#include "Components/TextRenderer/TextRenderer.h"
#include "Components/Transform/Transform.h"
#include "Components/WorldEnvironment/WorldEnvironment.h"

#include "Components/Animator/Animator.h"
#include "Components/PhysicsBody/PhysicsBody.h"
#include "Components/Particles/ParticleSystem.h"
#include "Components/Tag/Tag.h"
#include "Components/Timer/Timer.h"

#include "Components/Control/Control.h"
#include "Components/Control/Button/Button.h"

#include "Components/PlayerController/PlayerController.h"
#include "Components/ThirdPersonCamera/ThirdPersonCamera.h"

#include "Components/AISystem/AIAgent/AIAgent.h"
#include "Components/AISystem/AISystem.h"
#include "Components/AISystem/NavigationMesh/NavigationMesh.h"
#include "Components/AISystem/NavigationTarget/NavigationTarget.h"
#include "Components/Hitbox/Hitbox.h"


#include "Material/Material.h"
#include "Shader/Shader.h"

#endif // !SERIALIZETYPES_H