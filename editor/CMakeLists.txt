add_executable(DeepEngine WIN32 main.cpp)
target_link_libraries(DeepEngine PRIVATE
	EngineJobs EngineConsole EngineMath EngineMemory EnginePlatform
	EngineRender EnginePhysics
	EnTT::EnTT ImGui
	d3d11 d3dcompiler dxguid winmm comdlg32
)
