add_library(EngineRender STATIC
	RenderSystem.cpp
	Camera.cpp
	Mesh.cpp
)
target_include_directories(EngineRender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EngineRender PUBLIC EngineMath EnginePlatform EngineConsole d3d11 d3dcompiler)
