add_library(EngineConsole STATIC
	Logger.cpp
)
target_include_directories(EngineConsole PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EngineConsole PUBLIC EnginePlatform)
