# Always provide EOF, even when CTest inherits an interactive console.
file(MAKE_DIRECTORY "${WORK_DIRECTORY}")
set(input_path "${WORK_DIRECTORY}/empty_input.txt")
file(WRITE "${input_path}" "")
execute_process(
    COMMAND "${CALC_EXECUTABLE}"
    INPUT_FILE "${input_path}"
    OUTPUT_VARIABLE output ERROR_VARIABLE errors RESULT_VARIABLE status TIMEOUT 5
)
if(NOT "${status}" STREQUAL "0" OR NOT output STREQUAL "" OR NOT errors STREQUAL "")
    message(FATAL_ERROR "Startup with EOF failed: status=${status}, output=[${output}], errors=[${errors}]")
endif()
