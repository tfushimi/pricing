pybind11_add_module(pypricing
        ${CMAKE_CURRENT_SOURCE_DIR}/src/RegisterBindings.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Bindings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/BindObservables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/BindPayoffs.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/BindMarket.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/BindPricer.cpp
)
target_link_libraries(pypricing PRIVATE pricer payoff market)
target_include_directories(pypricing PRIVATE ${CMAKE_SOURCE_DIR}/libs/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
