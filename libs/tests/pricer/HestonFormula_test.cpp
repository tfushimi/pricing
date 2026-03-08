#include "pricer/BSFormula.h"
#include "pricer/HestonFormula.h"

#include <gtest/gtest.h>

using namespace pricer;

constexpr double F = 105.0;
constexpr double K = 100.0;
constexpr double T = 1.0;
constexpr double r = 0.05;
const double dF = std::exp(-r * T);

TEST(HestonFormulaTest, ReducesToBS) {
    constexpr double v0=0.04, kappa=3.0, theta=0.04, xi=1e-4, rho=0.0;
    const double hestonPrice = hestonCallFormula(F, K, T, dF, v0, kappa, theta, xi, rho);
    const double bsPrice = blackCallFormula(F, K, T, dF, std::sqrt(v0));
    EXPECT_NEAR(hestonPrice, bsPrice, 1e-3);
}

// TODO add more unit tests