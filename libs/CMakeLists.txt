add_library(common INTERFACE)
add_library(numerics STATIC
        src/numerics/linear/PiecewiseLinearFunction.cpp
)
add_library(payoff STATIC
        src/payoff/ConstantFold.cpp
        src/payoff/MarketVisitor.cpp
        src/payoff/FixingCollector.cpp
)
add_library(market INTERFACE)
add_library(pricer STATIC src/pricer/BSPricer.cpp)

# export headers
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(numerics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(payoff
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src       # exposes src/ only to own .cpp files
)
target_include_directories(market INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(pricer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(numerics PUBLIC
        project_warnings
)

target_link_libraries(payoff INTERFACE
        numerics
        project_warnings
)

# tests
add_subdirectory(tests)