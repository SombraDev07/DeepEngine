cmake_minimum_required(VERSION 3.20)
project(DeepEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# --- EnTT (ECS, header-only) ---
FetchContent_Declare(EnTT
	GIT_REPOSITORY  https://github.com/skypjack/entt.git
	GIT_TAG         v3.13.2
	GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(EnTT)
message(STATUS "EnTT: ${entt_SOURCE_DIR}")

# --- Dear ImGui (Editor UI) ---
FetchContent_Declare(imgui
	GIT_REPOSITORY  https://github.com/ocornut/imgui.git
	GIT_TAG         v1.91.0
	GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(ImGui STATIC
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_demo.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)
target_include_directories(ImGui PUBLIC
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends
)
target_link_libraries(ImGui PUBLIC d3d11 d3dcompiler)
message(STATUS "ImGui: ${imgui_SOURCE_DIR}")

# --- Jolt Physics ---
# --- Jolt Physics (downloaded via FetchContent, built via add_subdirectory in physics/) ---
FetchContent_Declare(JoltPhysics
	GIT_REPOSITORY  https://github.com/jrouwe/JoltPhysics.git
	GIT_TAG         v5.0.0
	GIT_SHALLOW     TRUE
)
FetchContent_GetProperties(JoltPhysics)
if(NOT joltphysics_POPULATED)
	FetchContent_Populate(JoltPhysics)
endif()
message(STATUS "JoltPhysics: ${joltphysics_SOURCE_DIR}")

# --- MiniAudio (header-only) ---
FetchContent_Declare(miniaudio
	GIT_REPOSITORY  https://github.com/mackron/miniaudio.git
	GIT_TAG         master
	GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(miniaudio)
message(STATUS "MiniAudio: ${miniaudio_SOURCE_DIR}")

option(BUILD_TESTS "Build test programs" ON)
option(BUILD_EDITOR "Build the engine editor" ON)

add_subdirectory(core)
add_subdirectory(physics)
add_subdirectory(render)

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()

if(BUILD_EDITOR)
	add_subdirectory(editor)
endif()
