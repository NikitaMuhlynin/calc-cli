foreach(required_file IN ITEMS CALC_EXECUTABLE PLUGIN_LIBRARY)
    if(NOT DEFINED ${required_file} OR NOT EXISTS "${${required_file}}")
        message(FATAL_ERROR "Missing build artifact: ${required_file}=${${required_file}}")
    endif()
endforeach()

get_filename_component(calc_directory "${CALC_EXECUTABLE}" DIRECTORY)
get_filename_component(plugin_directory "${PLUGIN_LIBRARY}" DIRECTORY)
get_filename_component(calc_name "${CALC_EXECUTABLE}" NAME)
get_filename_component(plugin_extension "${PLUGIN_LIBRARY}" LAST_EXT)

if(NOT calc_name STREQUAL "calc.exe")
    message(FATAL_ERROR "Expected the executable to be named calc.exe")
endif()

if(NOT plugin_directory STREQUAL "${calc_directory}/plugins")
    message(FATAL_ERROR "Expected plugin DLLs in the plugins directory beside calc.exe")
endif()

if(NOT plugin_extension STREQUAL ".dll")
    message(FATAL_ERROR "Expected a plugin DLL")
endif()
