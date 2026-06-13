cmake_minimum_required(VERSION 3.20)
project(CryLauncher)

set(CRYENGINE_DIR "C:/CRYENGINE_Source-release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CRYENGINE_DIR}/bin/win_x64")

include(FetchContent)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(imgui)
add_library(ImGui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp)
target_include_directories(ImGui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(ImGui PUBLIC d3d11 d3dcompiler)

add_executable(CryLauncher WIN32 src/main.cpp)
target_link_libraries(CryLauncher PRIVATE ImGui d3d11 d3dcompiler dxguid winmm)
# Link against CrySystem import library
target_link_libraries(CryLauncher PRIVATE "${CRYENGINE_DIR}/solutions_cmake/win64/Code/CryEngine/CrySystem/Debug/CrySystem.lib")
target_include_directories(CryLauncher PRIVATE "${CRYENGINE_DIR}/Code/CryEngine/CryCommon")
