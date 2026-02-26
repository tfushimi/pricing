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

add_executable(test_constant_fold_visitor payoff/ConstantFoldVisitor_test.cpp)
target_link_libraries(test_constant_fold_visitor PRIVATE
        payoff
        GTest::gtest_main
)

add_executable(test_payoff_node payoff/PayoffNode_test.cpp)
target_link_libraries(test_payoff_node PRIVATE
        payoff
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_plf)
gtest_discover_tests(test_bs_formula)
gtest_discover_tests(test_constant_fold_visitor)
gtest_discover_tests(test_payoff_node)