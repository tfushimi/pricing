#include "numerics/RootFinding.h"

#include <gtest/gtest.h>

#include <cmath>

using namespace numerics::rootfinding;

TEST(RootFindingTest, LinearFunction) {
    // f(x) = x, target = 0 -> solution = 0
    const double sol = bisection(0.0, -1.0, 1.0, 1e-8, [](double x) { return x; });
    EXPECT_NEAR(sol, 0.0, 1e-8);
}

TEST(RootFindingTest, Quadratic) {
    // f(x) = x^2, target = 4 -> solution = 2
    const double sol = bisection(4.0, 0.0, 10.0, 1e-8, [](double x) { return x * x; });
    EXPECT_NEAR(sol, 2.0, 1e-6);
}

TEST(RootFindingTest, Exponential) {
    // f(x) = exp(x), target = 1 -> solution = 0
    const double sol =
        bisection(1.0, -1.0, 1.0, 1e-8, [](double x) { return std::exp(x); });
    EXPECT_NEAR(sol, 0.0, 1e-6);
}
