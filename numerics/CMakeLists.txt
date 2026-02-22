add_library(numerics INTERFACE
        types.h
        Segment.h
        PiecewiseLinearFunction.h
)

add_executable(test_pwl test_pwl.cpp)