add_executable(test_plf numerics/linear/test_plf.cpp)
target_link_libraries(test_plf PRIVATE
        numerics
        GTest::gtest_main
)

add_executable(test_bsformula models/test_bsformula.cpp)
target_link_libraries(test_bsformula PRIVATE
        models
        GTest::gtest_main
)
#add_executable(test_payoff test_payoff.cpp)
#target_link_libraries(test_payoff PRIVATE
#        payoff
#        GTest::gtest_main
#)
#
include(GoogleTest)
gtest_discover_tests(test_plf)
gtest_discover_tests(test_bsformula)