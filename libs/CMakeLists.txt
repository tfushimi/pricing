add_library(numerics STATIC
        src/numerics/linear/PiecewiseLinearFunction.cpp
)
add_library(payoff INTERFACE)
add_library(market INTERFACE)
add_library(models INTERFACE)

# export headers
target_include_directories(numerics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(payoff INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(market INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(models INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(numerics PUBLIC
        project_warnings
)

target_link_libraries(payoff INTERFACE
        numerics
        project_warnings
)

# tests
add_subdirectory(tests)