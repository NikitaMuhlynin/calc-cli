add_library(calc_core STATIC parser.cpp evaluator.cpp)
target_include_directories(calc_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(calc main.cpp console.cpp)
target_link_libraries(calc PRIVATE calc_core)
