add_library(EnginePhysics STATIC
	PhysicsSystem.cpp
)
target_include_directories(EnginePhysics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(EnginePhysics PUBLIC ${joltphysics_SOURCE_DIR})
target_link_libraries(EnginePhysics PUBLIC EngineMath EnginePlatform EngineConsole EnTT::EnTT Jolt)
