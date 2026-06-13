	add_library(EngineRender STATIC
		RenderSystem.cpp
		Camera.cpp
		Mesh.cpp
		cgltf_impl.cpp
		Terrain.cpp
	)
	target_include_directories(EngineRender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/thirdparty/cgltf")
target_link_libraries(EngineRender PUBLIC EngineMath EnginePlatform EngineConsole d3d11 d3dcompiler)
