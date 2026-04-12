add_executable(test_integration numerics/Integration_test.cpp)
target_link_libraries(test_integration PRIVATE
        numerics
        GTest::gtest_main
)

add_executable(test_root_finding numerics/RootFinding_test.cpp)
target_link_libraries(test_root_finding PRIVATE
        numerics
        GTest::gtest_main
)

add_executable(test_plf numerics/linear/PiecewiseLinearFunction_test.cpp)
target_link_libraries(test_plf PRIVATE
        numerics
        GTest::gtest_main
)

add_executable(test_bs_formula pricer/BSFormula_test.cpp)
target_link_libraries(test_bs_formula PRIVATE
        pricer
        GTest::gtest_main
)

add_executable(test_heston_formula pricer/HestonFormula_test.cpp)
target_link_libraries(test_heston_formula PRIVATE
        pricer
        numerics
        GTest::gtest_main
)

add_executable(test_plf_payoff_pricer pricer/PLFPayoffPricer_test.cpp)
target_link_libraries(test_plf_payoff_pricer PRIVATE
        payoff
        pricer
        GTest::gtest_main
)

add_executable(test_mc_pricer pricer/MCPricer_test.cpp)
target_link_libraries(test_mc_pricer PRIVATE
        payoff
        pricer
        GTest::gtest_main
)

add_executable(test_constant_fold payoff/ConstantFold_test.cpp)
target_link_libraries(test_constant_fold PRIVATE
        payoff
        GTest::gtest_main
)

add_executable(test_observable payoff/Observable_test.cpp)
target_link_libraries(test_observable PRIVATE
        payoff
        GTest::gtest_main
)

add_executable(test_apply_fixings payoff/ApplyFixings_test.cpp)
target_link_libraries(test_apply_fixings PRIVATE
        payoff
        GTest::gtest_main
)

add_executable(test_apply_market payoff/ApplyMarket_test.cpp)
target_link_libraries(test_apply_market PRIVATE
        payoff
        market
        GTest::gtest_main
)

add_executable(test_fixing_collector payoff/FixingCollector_test.cpp)
target_link_libraries(test_fixing_collector PRIVATE
        payoff
        GTest::gtest_main
)

add_executable(test_json_encoder payoff/JsonEncoder_test.cpp)
target_link_libraries(test_json_encoder PRIVATE
        payoff
        GTest::gtest_main
)

add_executable(test_process mc/Process_test.cpp)
target_link_libraries(test_process PRIVATE
        mc
        GTest::gtest_main
)

add_executable(test_process_state_stepper mc/ProcessStateStepper_test.cpp)
target_link_libraries(test_process_state_stepper PRIVATE
        mc
        GTest::gtest_main
)

add_executable(test_time_grid mc/TimeGrid_test.cpp)
target_link_libraries(test_time_grid PRIVATE
        mc
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_integration)
gtest_discover_tests(test_root_finding)
gtest_discover_tests(test_plf)
gtest_discover_tests(test_bs_formula)
gtest_discover_tests(test_heston_formula)
gtest_discover_tests(test_plf_payoff_pricer)
gtest_discover_tests(test_mc_pricer)
gtest_discover_tests(test_constant_fold)
gtest_discover_tests(test_observable)
gtest_discover_tests(test_apply_fixings)
gtest_discover_tests(test_apply_market)
gtest_discover_tests(test_fixing_collector)
gtest_discover_tests(test_json_encoder)
gtest_discover_tests(test_process)
gtest_discover_tests(test_process_state_stepper)
gtest_discover_tests(test_time_grid)