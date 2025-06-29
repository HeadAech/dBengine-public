if (MSVC)
    add_compile_options(/bigobj)
endif()

# Enable debug symbols in Release builds
if (MSVC)
    # For Visual Studio
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
else()
    # For GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
endif()

# Optional: Show flags to verify
message(STATUS "C++ Release Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Linker Release Flags: ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	 *.c
	 *.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	 *.h
	 *.hpp)
	 
# Search for the assets files
file(GLOB_RECURSE ASSETS_FILES 
	 ${CMAKE_SOURCE_DIR}/res/*.*)

set_source_files_properties(${ASSETS_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})						   
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR}         FILES ${ASSETS_FILES})

#find_package(Lua REQUIRED)
#find_package(LuaJIT REQUIRED)
#include_directories(${LUA_INCLUDE_DIR})
# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${ASSETS_FILES}
		GameObject/GameObject.cpp
		GameObject/GameObject.h
		Component/Component.cpp
		Component/Component.h
		Components/Transform/Transform.cpp
		Components/Transform/Transform.h
		Scene/Scene.cpp
		Scene/Scene.h
		Components/MeshInstance/MeshInstance.cpp
		Components/MeshInstance/MeshInstance.h
		Signal/Signal.h
		Shader/Shader.cpp
		Shader/Shader.h
		Components/Camera/Camera.cpp
		Components/Camera/Camera.h
		Helpers/Util.h
		InputManager/Input.cpp
		InputManager/Input.h
		Components/LuaComponent/LuaComponent.cpp
		Components/LuaComponent/LuaComponent.h
		Signal/Signals.cpp
		Signal/Signals.h
		EngineGUI/EngineGUI.cpp
		EngineGUI/EngineGUI.h
		Components/LuaComponent/Lua_GameObject/Lua_GameObject.cpp
		Components/LuaComponent/Lua_GameObject/Lua_GameObject.h
		Components/LuaComponent/Lua.h
		dBrender/dBrender.cpp
		dBrender/dBrender.h
		Components/CollisionShape/CollisionShape.cpp
		Components/CollisionShape/CollisionShape.h
		Components/AudioSource/AudioSource.cpp
		Components/AudioSource/AudioSource.h
		Components/AudioListener/AudioListener.cpp
		Components/AudioListener/AudioListener.h
		Components/LuaComponent/Lua_Input/Lua_Input.cpp
		Components/LuaComponent/Lua_Input/Lua_Input.h
		Components/LuaComponent/Lua_Camera/Lua_Camera.cpp
		Components/LuaComponent/Lua_Camera/Lua_Camera.h
		dBrender/dBrender.cpp
		dBrender/dBrender.h
		dBengine/dBengine.cpp
		dBengine/dBengine.h
		dBengine/EngineSettings/EngineSettings.cpp
		dBengine/EngineSettings/EngineSettings.h
		dBphysics/dBphysics.cpp
		dBphysics/dBphysics.h
		dBengine/EngineDebug/EngineDebug.cpp
		dBengine/EngineDebug/EngineDebug.h 
		dBrender/Framebuffer/Framebuffer.h
		dBrender/Framebuffer/Framebuffer.cpp 
		Helpers/UUID/UUID.h
		Components/TextRenderer/TextRenderer.cpp
		Components/TextRenderer/TextRenderer.h
		Components/LuaComponent/Lua_TextRenderer/Lua_TextRenderer.cpp
		Components/LuaComponent/Lua_TextRenderer/Lua_TextRenderer.h 
		Gizmo/Gizmo.cpp
		Gizmo/Gizmo.h
		Components/Lights/Light.h
		Components/Lights/Light.cpp
		Components/Lights/DirectionalLight/DirectionalLight.h
		Components/Lights/DirectionalLight/DirectionalLight.cpp
		Components/Lights/PointLight/PointLight.h 
		Components/Lights/PointLight/PointLight.cpp
		Components/Lights/SpotLight/SpotLight.h
		Components/Lights/SpotLight/SpotLight.cpp 
		Components/Lights/LocalLight.h 
		Components/Lights/LocalLight.cpp
		dBrender/ShadowMapFramebuffer/ShadowMapBuffer.h
		dBrender/ShadowMapFramebuffer/ShadowMapBuffer.cpp
		dBrender/ShadowMapFramebuffer/ShadowCubeMapBuffer/ShadowCubeMapBuffer.h
		dBrender/ShadowMapFramebuffer/ShadowCubeMapBuffer/ShadowCubeMapBuffer.cpp
		Helpers/fonts/IconsFontAwesome4.h
			EngineGUI/EngineTheme.h 
			imgui_impl/imgui_spectrum.h
		dBrender/FramebufferMultisample/FramebufferMultisample.h
		dBrender/FramebufferMultisample/FramebufferMultisample.cpp
		EngineGUI/FileExplorer/FileExplorer.h
		EngineGUI/FileExplorer/FileExplorer.cpp
		Material/Material.h
		Material/Material.cpp
		Components/WorldEnvironment/WorldEnvironment.h
		Components/WorldEnvironment/WorldEnvironment.cpp
		Serializers/SceneSerializer.h 
		Serializers/SceneSerializer.cpp 
		Serializers/SerializeTypes.h 
		dBrender/PostProcessing/Bloom/BloomRenderer.cpp
		dBrender/PostProcessing/Bloom/BloomRenderer.h
		Frustum/Frustum.h
		Frustum/Plane.h
		BoundingVolume/BoundingVolume.h
		BoundingVolume/SphereBoundingVolume.h
		BoundingVolume/AABB.h
		BoundingVolume/SquareAABB.h "Components/MeshInstance/Mesh.h" "EngineGUI/Panel/Panel.h" "EngineGUI/Panel/Panel.cpp" "EngineGUI/Panel/Panels/ViewportPanel/ViewportPanel.h" "EngineGUI/Panel/Panels/ViewportPanel/ViewportPanel.cpp" "EngineGUI/Panel/Panels/InspectorPanel/InspectorPanel.h" "EngineGUI/Panel/Panels/InspectorPanel/InspectorPanel.cpp" "EngineGUI/Panel/Panels/ConsolePanel/ConsolePanel.h" "EngineGUI/Panel/Panels/ConsolePanel/ConsolePanel.cpp" "EngineGUI/Panel/Panels/DebugPanel/DebugPanel.h" "EngineGUI/Panel/Panels/DebugPanel/DebugPanel.cpp" "EngineGUI/Panel/Panels/ScenePanel/ScenePanel.h" "EngineGUI/Panel/Panels/ScenePanel/ScenePanel.cpp" "EngineGUI/Panel/Panels/SettingsPanel/SettingsPanel.h" "EngineGUI/Panel/Panels/SettingsPanel/SettingsPanel.cpp" "EngineGUI/Panel/Panels/CodeEditorPanel/CodeEditorPanel.h" "EngineGUI/Panel/Panels/CodeEditorPanel/CodeEditorPanel.cpp" "EngineGUI/Panel/Panels/ScenePanel/CreatePanel/CreatePanel.h" "EngineGUI/Panel/Panels/ScenePanel/CreatePanel/CreatePanel.cpp" "EngineGUI/Panel/Panels/EditorMenuPanel/EditorMenuPanel.h" "EngineGUI/Panel/Panels/EditorMenuPanel/EditorMenuPanel.cpp" "Helpers/Colors/Colors.h" "ResourceManager/ResourceManager.h" "ResourceManager/ResourceManager.cpp" "Components/Animator/Animation/Animation.h" "Components/Animator/Bone/Bone.h" "Components/Animator/Bone/Bone.cpp" "Components/Animator/Animation/Animation.cpp" "Components/Animator/Animator.cpp" "Components/Animator/AnimationLibrary/AnimationLibrary.h"
		"Components/Animator/AnimationLibrary/AnimationLibrary.cpp" "EngineGUI/Panel/Panels/AnimationLibraryPanel/AnimationLibraryPanel.h" "EngineGUI/Panel/Panels/AnimationLibraryPanel/AnimationLibraryPanel.cpp" "EngineGUI/Panel/Panels/AnimationTransitionPanel/AnimationTransitionPanel.h" "EngineGUI/Panel/Panels/AnimationTransitionPanel/AnimationTransitionPanel.cpp" "Components/LuaComponent/Lua_Animator/Lua_Animator.h" "Components/LuaComponent/Lua_Animator/Lua_Animator.cpp" "EngineGUI/Panel/Panels/InputManagerPanel/InputManagerPanel.h" "EngineGUI/Panel/Panels/InputManagerPanel/InputManagerPanel.cpp" "EngineGUI/Panel/Panels/ThemeEditorPanel/ThemeEditorPanel.h" "EngineGUI/Panel/Panels/ThemeEditorPanel/ThemeEditorPanel.cpp" "EngineGUI/Font/Font.h" "EngineGUI/Panel/Panels/InspectorPanel/AddComponentPanel/AddComponentPanel.h" "EngineGUI/Panel/Panels/InspectorPanel/AddComponentPanel/AddComponentPanel.cpp" "Singletons/Clipboard/Clipboard.h" "Serializers/MaterialSerializer.cpp" "Serializers/MaterialSerializer.h" "Components/Particles/ParticleSystem.h" "Components/Particles/ParticleSystem.cpp" "Helpers/Random.h" "Helpers/DebugDraw.h"
			"Components/LuaComponent/Lua_ParticleSystem/Lua_ParticleSystem.h" "Components/LuaComponent/Lua_ParticleSystem/Lua_ParticleSystem.cpp"
		 "EngineGUI/Panel/Panels/PreviewPanel/PreviewPanel.h" "EngineGUI/Panel/Panels/PreviewPanel/PreviewPanel.cpp" "Helpers/TimerHelper/TimerHelper.h" "Components/MeshInstance/Model.h" "Components/MeshInstance/Model.cpp" "Components/Tag/Tag.h" "Components/Tag/Tag.cpp" "EngineGUI/Panel/Panels/InspectorPanel/MaterialPanel/MaterialPanel.h" "EngineGUI/Panel/Panels/InspectorPanel/MaterialPanel/MaterialPanel.cpp" "dBphysics/Octree/Octree.h" "Components/PhysicsBody/PhysicsBody.h" "Components/PhysicsBody/PhysicsBody.cpp" "EngineGUI/Panel/Panels/ScenePanel/AppendPanel/AppendPanel.cpp" "EngineGUI/Panel/Panels/ScenePanel/AppendPanel/AppendPanel.h" "Serializers/SerializeEmitters.h" 		  "Components/Control/Control.h" "Components/Control/Control.cpp" "Components/Control/Button/Button.h" "Components/Control/Button/Button.cpp" "Singletons/Ref/Ref.h" "Singletons/Ref/Ref.cpp"
		 "Components/MeshInstance/GenericMesh.h" "Components/MeshInstance/GenericMesh.cpp"  "Components/Timer/Timer.h" "Components/ThirdPersonCamera/ThirdPersonCamera.h" "Components/ThirdPersonCamera/ThirdPersonCamera.cpp" "Components/PlayerController/PlayerController.h" "Components/PlayerController/PlayerController.cpp"
		Components/AISystem/NavigationMesh/NavigationMesh.cpp
		Components/AISystem/NavigationMesh/NavigationMesh.h
		Components/AISystem/AIAgent/AIAgent.cpp
		Components/AISystem/AIAgent/AIAgent.h
		Components/AISystem/AISystem.cpp
		Components/AISystem/AISystem.h
		Components/AISystem/NavigationTarget/NavigationTarget.cpp
		Components/AISystem/NavigationTarget/NavigationTarget.h
 "Signal/LuaSignal.h" "Components/Hitbox/Hitbox.h" "Components/Hitbox/Hitbox.cpp" "Components/Control/Text/Text.h" "Components/Control/Text/Text.cpp"
		Components/Control/Sprite/Sprite.h
 
  "Components/Control/Sprite/Sprite.cpp"
  dBrender/PostProcessing/Shake/CameraShake.h
   "dBrender/PostProcessing/Fade/FadeTransition.h" "Scheduler/Scheduler.h" "Scheduler/Scheduler.cpp")

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
												  ${glad_SOURCE_DIR}
												  ${stb_image_SOURCE_DIR}
												  ${imgui_SOURCE_DIR}
													${SOL_DIR}
													${FMOD_INCLUDE_DIR}
		${FREETYPE_DIR}
		${FREETYPE_SOURCE_DIR}
		${FREETYPE_INCLUDE_DIR}
		${FREETYPE_BUILDS_DIR}
		${ImGuiFileDialog_SOURCE_DIR}
													)
                          
#target_link_libraries(${PROJECT_NAME} /opt/local/lib/libluajit-5.1.2.dylib)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} stb_image)
target_link_libraries(${PROJECT_NAME} assimp)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} spdlog)
target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} lua)
#target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARY})
#target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARIES})
target_link_libraries(${PROJECT_NAME} sol)
target_link_libraries(${PROJECT_NAME} fmod)
target_link_libraries(${PROJECT_NAME} freetype)
target_link_libraries(${PROJECT_NAME} ImGuiFileDialog)
target_link_libraries(${PROJECT_NAME} yaml-cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${imgui_SOURCE_DIR}
  ${ImGuiFileDialog_SOURCE_DIR}
)

target_sources(${PROJECT_NAME} PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${ImGuiFileDialog_SOURCE_DIR}/ImGuiFileDialog.cpp
)
#message(STATUS "LUA_INCLUDE_DIR: ${LUA_INCLUDE_DIR}")
#message(STATUS "LUA_LIBRARY: ${LUA_LIBRARY}")

message(STATUS "FMOD_DLLS: ${FMOD_DLLS}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
            ${CMAKE_SOURCE_DIR}/res 
            ${CMAKE_CURRENT_BINARY_DIR}/res
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FMOD_DLLS} ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copying FMOD DLLs to output directory.")

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()