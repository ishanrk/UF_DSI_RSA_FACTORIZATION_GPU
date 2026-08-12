set(CADO_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/cado-nfs" CACHE PATH "CADO source checkout")
set(CADO_BINARY_DIR "${CMAKE_BINARY_DIR}/cado" CACHE PATH "CADO CPU build directory")
set(CADO_CMAKE_ARGS "" CACHE STRING "Additional CADO CMake arguments, separated by semicolons")

if(NOT EXISTS "${CADO_SOURCE_DIR}/cado.h")
    message(FATAL_ERROR "CADO is missing; run git submodule update --init third_party/cado-nfs")
endif()
execute_process(COMMAND ${GIT_EXECUTABLE} -C ${CADO_SOURCE_DIR} rev-parse HEAD
    RESULT_VARIABLE cado_git_status
    OUTPUT_VARIABLE cado_revision OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT cado_git_status EQUAL 0 OR NOT cado_revision STREQUAL "692ecb7e62f0f3bdab88ee44cc60b8ded0ea1a1b")
    message(FATAL_ERROR "CADO checkout does not match the pinned revision 692ecb7e62f0f3bdab88ee44cc60b8ded0ea1a1b")
endif()
message(STATUS "CADO revision ${cado_revision}")

# CADO uses its own project-wide feature checks and configuration headers.
# These CPU targets do not run the coordinator, which also needs Flask.
include(ExternalProject)
ExternalProject_Add(cado_cpu
    SOURCE_DIR "${CADO_SOURCE_DIR}"
    BINARY_DIR "${CADO_BINARY_DIR}"
    DOWNLOAD_COMMAND "" UPDATE_COMMAND "" PATCH_COMMAND ""
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env NO_PYTHON_CHECK=1 ${CMAKE_COMMAND}
        -S "${CADO_SOURCE_DIR}" -B "${CADO_BINARY_DIR}" -G "${CMAKE_GENERATOR}"
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        ${CADO_CMAKE_ARGS}
    BUILD_COMMAND ${CMAKE_COMMAND} --build "${CADO_BINARY_DIR}"
    INSTALL_COMMAND ""
    STEP_TARGETS configure
    EXCLUDE_FROM_ALL TRUE)

foreach(program polyselect polyselect_ropt makefb las dup1 dup2 purge merge replay sqrt)
    add_custom_target(cado-${program}
        COMMAND ${CMAKE_COMMAND} --build ${CADO_BINARY_DIR} --target ${program}
        DEPENDS cado_cpu-configure
        USES_TERMINAL)
endforeach()
