find_program(CALC_CPPCHECK_EXECUTABLE NAMES cppcheck)
if(CALC_CPPCHECK_EXECUTABLE)
    add_custom_target(lint-cppcheck
        COMMAND "${CALC_CPPCHECK_EXECUTABLE}"
            --quiet --error-exitcode=1 --inline-suppr
            --enable=warning,style,performance,portability
            --std=c++17 --platform=win64 --library=googletest
            # This constructor-free record is also compiled as C11. Its users
            # value-initialize it before passing it across the DLL boundary.
            "--suppress=uninitMemberVarNoCtor:*/include/calc/plugin_api.h"
            "-I${PROJECT_SOURCE_DIR}/include" "-I${PROJECT_SOURCE_DIR}/src"
            "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/plugins"
            "${PROJECT_SOURCE_DIR}/tests"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM USES_TERMINAL
    )
endif()

find_program(CALC_CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
if(CALC_CLANG_TIDY_EXECUTABLE AND CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(lint-clang-tidy
        COMMAND "${CMAKE_COMMAND}"
            "-DCLANG_TIDY=${CALC_CLANG_TIDY_EXECUTABLE}"
            "-DBUILD_DIRECTORY=${PROJECT_BINARY_DIR}"
            "-DSOURCE_DIRECTORY=${PROJECT_SOURCE_DIR}"
            -P "${PROJECT_SOURCE_DIR}/cmake/RunClangTidy.cmake"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM USES_TERMINAL
    )
endif()
