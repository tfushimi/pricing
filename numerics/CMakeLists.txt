add_library(numerics STATIC
        types.h
        Segment.h
        PiecewiseLinearFunction.h
        PiecewiseLinearFunction.cpp
)

target_include_directories(numerics INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..   # exposes numerics/ as a subdirectory
)

add_executable(test_pwl test_pwl.cpp)
target_link_libraries(test_pwl GTest::gtest_main)