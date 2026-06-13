add_executable(EngineTest main.cpp ecs_test.cpp)
target_link_libraries(EngineTest PRIVATE EngineJobs EngineConsole EngineMath EngineMemory EnginePlatform EnTT::EnTT)
