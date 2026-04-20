add_executable(json_example JsonExample.cpp)
target_link_libraries(json_example PRIVATE pricer project_warnings)
configure_file(payoff.json ${CMAKE_CURRENT_BINARY_DIR}/payoff.json COPYONLY)
