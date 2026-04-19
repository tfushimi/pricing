cmake_minimum_required(VERSION 3.16)
project(pricing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# fetch dependencies
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_Declare(
        pybind11
        URL https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest nlohmann_json pybind11)
enable_testing()

# project_warnings
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
        -Wall
        -Wextra
        -Wsign-compare
        -Wshadow
        -Wnon-virtual-dtor
        -Wpedantic
)

# libraries
add_subdirectory(libs)

# executable
add_subdirectory(app)

# python bindings
add_subdirectory(python)

# pytest
add_custom_target(
        pytest
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR}/python
        python3 -m pytest ${CMAKE_SOURCE_DIR}/python/tests -v
        DEPENDS pypricing
        COMMENT "Running Python tests"
        VERBATIM
)

# format
find_program(CLANG_FORMAT clang-format)

if(CLANG_FORMAT)
        add_custom_target(
                format
                COMMAND find
                ${CMAKE_SOURCE_DIR}/app
                ${CMAKE_SOURCE_DIR}/libs
                ${CMAKE_SOURCE_DIR}/python
        (-name "*.cpp" -o -name "*.h")
                -exec ${CLANG_FORMAT} -i {} +
                COMMENT "Running clang-format"
                VERBATIM
        )
endif()
