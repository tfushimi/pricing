add_executable(quant_finance main.cpp)
target_link_libraries(quant_finance PRIVATE numerics payoff pricer project_warnings)
