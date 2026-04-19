pybind11_add_module(pypricing RegisterBindings.h Bindings.cpp BindObservables.cpp BindPayoffs.cpp)
target_link_libraries(pypricing PRIVATE pricer payoff market)
target_include_directories(pypricing PRIVATE ${CMAKE_SOURCE_DIR}/libs/include)
