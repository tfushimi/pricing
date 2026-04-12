#include "numerics/Integration.h"

#include <gtest/gtest.h>

#include <cmath>

using namespace numerics::integration;

TEST(IntegrationTest, ZeroFunction) {
    EXPECT_NEAR(integrate([](double) { return 0.0; }), 0.0, 1e-10);
}

TEST(IntegrationTest, ExponentialDecay) {
    // integral of exp(-u) from 0 to inf = 1
    // the integrator starts at du=0.001 and goes to 200, so result is very close to 1
    EXPECT_NEAR(integrate([](double u) { return std::exp(-u); }), 1.0, 1e-2);
}

TEST(IntegrationTest, Gaussian) {
    // integral of exp(-u^2/2) / sqrt(2*pi) from 0 to inf = 0.5
    constexpr double sqrt2pi = 2.5066282746310002;
    EXPECT_NEAR(integrate([&](double u) { return std::exp(-u * u / 2.0) / sqrt2pi; }), 0.5, 1e-2);
}
