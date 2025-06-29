# CPM - package manager
include(CPM)
add_definitions(-DFT2_BUILD_LIBRARY)
# OpenGL
find_package(OpenGL REQUIRED)

# glad
set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/glad)
set(glad_SOURCE_DIR ${GLAD_DIR}/include CACHE INTERNAL "")
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)

# stb_image
set(STB_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb_image)
set(stb_image_SOURCE_DIR ${STB_IMAGE_DIR} CACHE INTERNAL "")
add_library(stb_image STATIC ${STB_IMAGE_DIR}/stb_image.cpp)
target_include_directories(stb_image PRIVATE ${STB_IMAGE_DIR})

#sol
set(SOL_DIR ${CMAKE_CURRENT_LIST_DIR}/sol)
set(sol_SOURCE_DIR ${SOL_DIR} CACHE INTERNAL "")
add_library(sol INTERFACE)
target_include_directories(sol INTERFACE ${SOL_DIR})
target_link_libraries(sol INTERFACE ${LUA_LIBRARY})

#freetype
#set(FREETYPE_DIR ${CMAKE_CURRENT_LIST_DIR}/freetype)
#set(FREETYPE_SOURCE_DIR ${FREETYPE_DIR}/src CACHE INTERNAL "")
#set(FREETYPE_INCLUDE_DIR ${FREETYPE_DIR}/include)
#set(FREETYPE_BUILDS_DIR ${FREETYPE_DIR}/builds)
#
## Create the static library
#add_library(freetype INTERFACE)
#target_include_directories(freetype INTERFACE ${FREETYPE_DIR})
#target_include_directories(freetype INTERFACE ${FREETYPE_INCLUDE_DIR})
#target_include_directories(freetype INTERFACE ${FREETYPE_BUILDS_DIR})

# Add FreeType as a subdirectory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/freetype)

# Include the FreeType headers
include_directories(${CMAKE_CURRENT_LIST_DIR}/freetype/include)

# Lua source directory (assuming it's at thirdparty/lua/src)
set(LUA_DIR ${CMAKE_CURRENT_LIST_DIR}/lua/src)

# Add Lua source files to the build
file(GLOB LUA_SOURCES
		${LUA_DIR}/*.c   # Or use a recursive search if needed
)

# Create a static library for Lua
add_library(lua STATIC ${LUA_SOURCES})

# Add the include directory for Lua headers
target_include_directories(lua PUBLIC ${LUA_DIR})

message(STATUS "Sol include path: ${SOL_DIR}")

# FMOD
set(FMOD_DIR ${CMAKE_CURRENT_LIST_DIR}/fmod/)
# Set the include and library directories
set(FMOD_INCLUDE_DIR ${FMOD_DIR}/inc CACHE INTERNAL "")
set(FMOD_LIBRARY_DIR ${FMOD_DIR}/lib CACHE INTERNAL "")

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_DIR "x64")
    else()
        set(ARCH_DIR "x86")
    endif()

    # Windows library paths
    set(FMOD_LIBRARIES
        ${FMOD_LIBRARY_DIR}/win/core/${ARCH_DIR}/fmod_vc.lib
        ${FMOD_LIBRARY_DIR}/win/studio/${ARCH_DIR}/fmodstudio_vc.lib
        ${FMOD_LIBRARY_DIR}/win/bank/${ARCH_DIR}/fsbank_vc.lib
    )

    file(GLOB FMOD_DLLS ${FMOD_LIBRARY_DIR}/win/dll/${ARCH_DIR}/*.dll)
elseif(APPLE)
    set(FMOD_LIBRARIES
        ${FMOD_LIBRARY_DIR}/mac/core/libfmod.dylib
        ${FMOD_LIBRARY_DIR}/mac/studio/libfmodstudio.dylib
        ${FMOD_LIBRARY_DIR}/mac/bank/libfsbank.dylib
    )


    set(FMOD_MAC
        ${FMOD_LIBRARY_DIR}/mac/core/*.dylib
        ${FMOD_LIBRARY_DIR}/mac/studio/*.dylib
        ${FMOD_LIBRARY_DIR}/mac/bank/*.dylib
    )
    file(GLOB FMOD_DLLS ${FMOD_MAC})
endif()

# For runtime DLLs
#file(GLOB FMOD_DLLS ${FMOD_LIBRARY_DIR}/win/dll/${ARCH_DIR}/*.dll)
set(FMOD_DLLS ${FMOD_DLLS} CACHE INTERNAL "FMOD DLL files for runtime")

# Create FMOD interface library
add_library(fmod INTERFACE)
target_include_directories(fmod INTERFACE ${FMOD_INCLUDE_DIR})
target_link_libraries(fmod INTERFACE ${FMOD_LIBRARIES})

# other
CPMAddPackage("gh:assimp/assimp@5.4.3")
CPMAddPackage("gh:glfw/glfw#3.3.8")
CPMAddPackage("gh:g-truc/glm#0.9.9.8")
#CPMAddPackage("gh:ocornut/imgui@1.88")
CPMAddPackage("gh:gabime/spdlog@1.10.0")
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG docking  # Use the docking branch
    OPTIONS
        "IMGUI_BUILD_EXAMPLES OFF"
        "IMGUI_BUILD_DEMO OFF"
        "IMGUI_BUILD_TESTS OFF"
)


CPMAddPackage(
      NAME ImGuiFileDialog
      GITHUB_REPOSITORY aiekick/ImGuiFileDialog
      GIT_TAG master
)

add_subdirectory(yaml-cpp)
set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "")

add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp
					     ${imgui_SOURCE_DIR}/imgui_demo.cpp
					     ${imgui_SOURCE_DIR}/imgui_draw.cpp
						 ${imgui_SOURCE_DIR}/imgui_tables.cpp
					     ${imgui_SOURCE_DIR}/imgui_widgets.cpp)


target_include_directories(ImGuiFileDialog PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

set_target_properties(glad
                      stb_image 
                      assimp 
                      glfw 
                      glm 
                      imgui
					  sol
		              lua
                      fmod
                      spdlog 
                      ImGuiFileDialog
                      PROPERTIES FOLDER "thirdparty")

if (TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER "thirdparty")
endif()