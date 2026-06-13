add_library(EngineCore INTERFACE)
target_include_directories(EngineCore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(platform)
add_subdirectory(math)
add_subdirectory(memory)
add_subdirectory(jobs)
add_subdirectory(console)
