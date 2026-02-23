cmake_minimum_required(VERSION 3.16)
project(quant_finance)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_subdirectory(numerics)
add_subdirectory(payoff)

add_executable(quant_finance main.cpp)
target_link_libraries(quant_finance PRIVATE numerics payoff)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

find_program(CLANG_FORMAT clang-format)

if(CLANG_FORMAT)
        add_custom_target(
                format
                COMMAND find
                ${CMAKE_SOURCE_DIR}/numerics
                ${CMAKE_SOURCE_DIR}/payoff
        (-name "*.cpp" -o -name "*.h")
                -exec ${CLANG_FORMAT} -i {} +
                COMMENT "Running clang-format"
                VERBATIM
        )
endif()
