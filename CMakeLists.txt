cmake_minimum_required(VERSION 3.24)

project(calc_cli VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "calc-cli currently targets Windows and DLL plugins")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The configuration expression prevents multi-config generators from appending
# another Debug/Release directory, keeping plugins beside the executable.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/$<CONFIG>")

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(calc_plugin_api INTERFACE)
target_include_directories(calc_plugin_api INTERFACE "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(src)
add_subdirectory(plugins)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
