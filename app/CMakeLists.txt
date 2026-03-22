add_executable(quant_finance main.cpp)
target_link_libraries(quant_finance PRIVATE pricer project_warnings)

add_executable(lcgf_cliquet lcgf_cliquet.cpp)
target_link_libraries(lcgf_cliquet PRIVATE pricer mc project_warnings)