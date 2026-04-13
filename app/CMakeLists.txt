add_subdirectory(gatheral)

add_executable(barrier_enhanced_note BarrierEnhancedNote.cpp)
target_link_libraries(barrier_enhanced_note PRIVATE pricer project_warnings)

add_executable(json_example JsonExample.cpp)
target_link_libraries(json_example PRIVATE pricer project_warnings)
configure_file(payoff.json ${CMAKE_CURRENT_BINARY_DIR}/payoff.json COPYONLY)
