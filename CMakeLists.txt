cmake_minimum_required(VERSION 3.16)
project(quant_finance)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

# fetch gtest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest)
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

# format
find_program(CLANG_FORMAT clang-format)

if(CLANG_FORMAT)
        add_custom_target(
                format
                COMMAND find
                ${CMAKE_SOURCE_DIR}/app
                ${CMAKE_SOURCE_DIR}/libds
        (-name "*.cpp" -o -name "*.h")
                -exec ${CLANG_FORMAT} -i {} +
                COMMENT "Running clang-format"
                VERBATIM
        )
endif()
