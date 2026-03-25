add_executable(quant_finance main.cpp)
target_link_libraries(quant_finance PRIVATE pricer project_warnings)

add_executable(lcgf_cliquet lcgf_cliquet.cpp)
target_link_libraries(lcgf_cliquet PRIVATE pricer project_warnings)

add_executable(reverse_cliquet reverse_cliquet.cpp)
target_link_libraries(reverse_cliquet PRIVATE pricer project_warnings)

add_executable(napoleon napoleon.cpp)
target_link_libraries(napoleon PRIVATE pricer project_warnings)

add_executable(one_touch_call one_touch_call.cpp)
target_link_libraries(one_touch_call PRIVATE pricer project_warnings)