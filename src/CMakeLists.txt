add_library(calc_core STATIC parser.cpp evaluator.cpp plugin_registry.cpp)
target_include_directories(calc_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(calc_core PUBLIC calc_plugin_api)

add_executable(calc main.cpp console.cpp)
target_link_libraries(calc PRIVATE calc_core)
