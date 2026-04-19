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
    constexpr HestonParams params{0.05, 5.0, 0.05, 0.5, -0.8};
    const double call = hestonCallFormula(F, K, T, dF, params);
    const double put = call - dF * (F - K);
    EXPECT_NEAR(call, 6.2528, 1e-3);
    EXPECT_NEAR(put, 5.7590, 1e-3);
}

TEST(HestonFormulaTest, CallReducesToBS) {
    constexpr HestonParams params{0.05, 5.0, 0.05, 1e-4, 0.0};
    const double hestonPrice = hestonCallFormula(F, K, T, dF, params);
    const double bsPrice = bsCallFormula(F, K, T, dF, std::sqrt(params.v0));
    EXPECT_NEAR(hestonPrice, bsPrice, 1e-3);
}

TEST(HestonFormulaTest, DigitalCallReducesToBS) {
    // xi -> 0 and rho = 0 means no skew
    constexpr HestonParams params{0.05, 5.0, 0.05, 1e-4, 0.0};
    const double hestonPrice = hestonDigitalCallFormula(F, K, T, dF, params);
    const double bsPrice = bsDigitalCallFormula(F, K, T, dF, std::sqrt(params.v0), 0.0);
    EXPECT_NEAR(hestonPrice, bsPrice, 1e-3);
}

TEST(HestonFormulaTest, DigitalCallWithSkew) {
    constexpr HestonParams params{0.05, 5.0, 0.05, 0.5, -0.8};

    constexpr double dK = 0.01;
    const double callUp = hestonCallFormula(F, K + dK, T, dF, params);
    const double callDown = hestonCallFormula(F, K - dK, T, dF, params);
    const double digitalFD = -(callUp - callDown) / (2.0 * dK);  // -dC/dK

    const double digitalFormula = hestonDigitalCallFormula(F, K, T, dF, params);

    EXPECT_NEAR(digitalFormula, digitalFD, 1e-3);
}

TEST(HestonFormulaTest, PutCallParity) {
    constexpr HestonParams params{0.05, 5.0, 0.05, 0.5, -0.8};
    const double call = hestonCallFormula(F, K, T, dF, params);
    const double put = hestonCallFormula(F, K, T, dF, params) - dF * (F - K);
    EXPECT_NEAR(call - put, dF * (F - K), 1e-10);
}

TEST(HestonFormulaTest, StrikeMonotonicity) {
    constexpr HestonParams params{0.05, 5.0, 0.05, 0.5, -0.8};
    const double c1 = hestonCallFormula(F, 90.0, T, dF, params);
    const double c2 = hestonCallFormula(F, 100.0, T, dF, params);
    const double c3 = hestonCallFormula(F, 110.0, T, dF, params);
    EXPECT_GT(c1, c2);
    EXPECT_GT(c2, c3);
}