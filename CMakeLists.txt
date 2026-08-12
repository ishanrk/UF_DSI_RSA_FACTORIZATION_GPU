cmake_minimum_required(VERSION 3.18)
project(gpu_nfs LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use an out-of-source build: cmake -S . -B build")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Git REQUIRED)
include(cmake/cado.cmake)

option(ENABLE_CUDA "Detect and enable the CUDA compiler" ON)
set(CMAKE_CUDA_ARCHITECTURES 100 CACHE STRING "CUDA architectures; B200 is 100")
if(ENABLE_CUDA)
    include(CheckLanguage)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER)
        find_package(CUDAToolkit REQUIRED)
        foreach(arch IN LISTS CMAKE_CUDA_ARCHITECTURES)
            if(arch MATCHES "^1[0-9][0-9]" AND CUDAToolkit_VERSION VERSION_LESS 12.8)
                message(FATAL_ERROR "Blackwell requires CUDA 12.8 or newer; select another architecture or use -DENABLE_CUDA=OFF")
            endif()
        endforeach()
        enable_language(CUDA)
        set(CMAKE_CUDA_STANDARD 20)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    else()
        message(STATUS "CUDA compiler not found; CPU reference targets remain available")
    endif()
endif()

option(BUILD_TESTING "Enable interface smoke tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
    add_test(NAME cado_interfaces
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/smoke/cado_interfaces.py
                --cado-source ${CADO_SOURCE_DIR})
    add_custom_target(cado-smoke
        COMMAND ${CMAKE_COMMAND} --build ${CADO_BINARY_DIR} --target las makefb
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/smoke/cado_interfaces.py
                --cado-source ${CADO_SOURCE_DIR} --cado-binary ${CADO_BINARY_DIR}
        DEPENDS cado_cpu-configure
        USES_TERMINAL)
endif()
