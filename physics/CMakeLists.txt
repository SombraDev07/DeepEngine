# Build Jolt from source
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
add_subdirectory("${joltphysics_SOURCE_DIR}/Build" "${joltphysics_BINARY_DIR}")

add_library(EnginePhysics STATIC
	PhysicsSystem.cpp
)
target_include_directories(EnginePhysics PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${joltphysics_SOURCE_DIR}
)
target_link_libraries(EnginePhysics PUBLIC EngineMath EnginePlatform EngineConsole EnTT::EnTT Jolt)
