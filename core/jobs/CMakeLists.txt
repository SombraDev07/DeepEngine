add_library(EngineJobs STATIC
	JobSystem.cpp
)
target_include_directories(EngineJobs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EngineJobs PUBLIC EnginePlatform EngineMemory)
