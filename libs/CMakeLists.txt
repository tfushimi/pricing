add_library(common INTERFACE)
add_library(numerics STATIC
        src/numerics/linear/PiecewiseLinearFunction.cpp
)
add_library(payoff STATIC
        src/payoff/ConstantFold.cpp
        src/payoff/ApplyMarket.cpp
        src/payoff/FixingCollector.cpp
        src/payoff/ToPiecewiseLinearFunction.cpp
)
add_library(market INTERFACE)
add_library(pricer STATIC
        src/pricer/BSPricer.cpp
)

# include directories
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(numerics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(payoff
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(market INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(pricer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# link libraries
target_link_libraries(common INTERFACE
        project_warnings
)
target_link_libraries(numerics PUBLIC
        common
)
target_link_libraries(payoff PUBLIC
        numerics
)
target_link_libraries(payoff PRIVATE
        project_warnings
)
target_link_libraries(market INTERFACE
        common
)
target_link_libraries(pricer PUBLIC
        numerics
        payoff
)
target_link_libraries(pricer PRIVATE
        project_warnings
)
add_subdirectory(tests)