add_library(common INTERFACE)
add_library(numerics STATIC
        src/numerics/linear/PiecewiseLinearFunction.cpp
)
add_library(payoff STATIC
        src/payoff/ConstantFold.cpp
        src/payoff/ApplyMarket.cpp
        src/payoff/FixingCollector.cpp
        src/payoff/ToPiecewiseLinearFunction.cpp
        src/payoff/ApplyFixings.cpp
        src/payoff/JsonCodec.cpp
)
add_library(market STATIC
        src/market/MarketDB.cpp
)
add_library(pricer STATIC
        src/pricer/PLFPayoffPricer.cpp
)
add_library(mc STATIC
        src/mc/Process.cpp
        src/mc/TimeGrid.cpp
)

# include directories
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(numerics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(payoff
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(market PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(pricer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(mc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# link libraries
target_link_libraries(common INTERFACE
        project_warnings
)
target_link_libraries(numerics PUBLIC
        common
)
target_link_libraries(payoff PUBLIC
        numerics
        nlohmann_json
        project_warnings
)
target_link_libraries(market PUBLIC
        common
        soci_core
        soci_sqlite3
        project_warnings
)
target_link_libraries(pricer PUBLIC
        numerics
        payoff
        mc
        project_warnings
)
target_link_libraries(mc PUBLIC
        market
        project_warnings
)

add_subdirectory(tests)