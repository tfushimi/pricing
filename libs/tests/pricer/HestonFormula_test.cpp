#include "pricer/HestonFormula.h"

#include <gtest/gtest.h>

#include "pricer/BSFormula.h"

using namespace pricer;

constexpr double T = 0.5;
constexpr double r = 0.03;
constexpr double q = 0.02;
const double dF = std::exp(-r * T);
const double F = 100.0 * std::exp((r - q) * T);
constexpr double K = 100.0;

TEST(HestonFormulaTest, Example) {
    // In p.15  of "The Heston Model and Its Extension in MATLAB and C#"
    constexpr double v0 = 0.05, kappa = 5.0, theta = 0.05, xi = 0.5, rho = -0.8;
    const double call = hestonCallFormula(F, K, T, dF, v0, kappa, theta, xi, rho);
    const double put = call - dF * (F - K);
    EXPECT_NEAR(call, 6.2528, 1e-3);
    EXPECT_NEAR(put, 5.7590, 1e-3);
}

TEST(HestonFormulaTest, CallReducesToBS) {
    constexpr double v0 = 0.05, kappa = 5.0, theta = 0.05, xi = 1e-4, rho = 0.0;
    const double hestonPrice = hestonCallFormula(F, K, T, dF, v0, kappa, theta, xi, rho);
    const double bsPrice = blackCallFormula(F, K, T, dF, std::sqrt(v0));
    EXPECT_NEAR(hestonPrice, bsPrice, 1e-3);
}

TEST(HestonFormulaTest, DigitalCallReducesToBS) {
    // xi -> 0 and rho = 0 means no skew
    constexpr double v0 = 0.05, kappa = 5.0, theta = 0.05, xi = 1e-4, rho = 0.0;
    const double hestonPrice = hestonDigitalCallFormula(F, K, T, dF, v0, kappa, theta, xi, rho);
    const double bsPrice = blackDigitalFormula(F, K, T, dF, std::sqrt(v0), 0.0);
    EXPECT_NEAR(hestonPrice, bsPrice, 1e-3);
}

TEST(HestonFormulaTest, DigitalCallWithSkew) {
    constexpr double v0 = 0.05, kappa = 5.0, theta = 0.05, xi = 0.5, rho = -0.8;

    constexpr double dK = 0.01;
    const double callUp = hestonCallFormula(F, K + dK, T, dF, v0, kappa, theta, xi, rho);
    const double callDown = hestonCallFormula(F, K - dK, T, dF, v0, kappa, theta, xi, rho);
    const double digitalFD = -(callUp - callDown) / (2.0 * dK);  // -dC/dK

    const double digitalFormula = hestonDigitalCallFormula(F, K, T, dF, v0, kappa, theta, xi, rho);

    EXPECT_NEAR(digitalFormula, digitalFD, 1e-3);
}

TEST(HestonFormulaTest, PutCallParity) {
    constexpr double v0 = 0.05, kappa = 5.0, theta = 0.05, xi = 0.5, rho = -0.8;
    const double call = hestonCallFormula(F, K, T, dF, v0, kappa, theta, xi, rho);
    const double put = hestonCallFormula(F, K, T, dF, v0, kappa, theta, xi, rho) - dF * (F - K);
    EXPECT_NEAR(call - put, dF * (F - K), 1e-10);
}

TEST(HestonFormulaTest, StrikeMonotonicity) {
    constexpr double v0 = 0.05, kappa = 5.0, theta = 0.05, xi = 0.5, rho = -0.8;
    const double c1 = hestonCallFormula(F, 90.0, T, dF, v0, kappa, theta, xi, rho);
    const double c2 = hestonCallFormula(F, 100.0, T, dF, v0, kappa, theta, xi, rho);
    const double c3 = hestonCallFormula(F, 110.0, T, dF, v0, kappa, theta, xi, rho);
    EXPECT_GT(c1, c2);
    EXPECT_GT(c2, c3);
}