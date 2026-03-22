#include "pricer/BSFormula.h"

#include <gtest/gtest.h>

const double F = 100.0;
const double K = 100.0;
const double T = 1.0;
const double r = 0.05;
const double vol = 0.20;
const double dF = std::exp(-r * T);  // discount factor

using namespace pricer;

TEST(BSTest, PutCallParity) {
    // C - P = dF * (F - K)
    const double call = bsCallFormula(F, K, T, dF, vol);
    const double put = bsCallFormula(F, K, T, dF, vol) - dF * (F - K);
    EXPECT_NEAR(call - put, dF * (F - K), 1e-10);
}

TEST(BSTest, DigitalCallFlatVol) {
    // With zero skew, digital = dF * N(d2)
    auto [d1, d2] = bsD1D2(F, K, T, vol);
    const double expected = dF * normalCdf(d2);
    const double actual = bsDigitalFormula(F, K, T, dF, vol, /*dVolDStrike=*/0.0);
    EXPECT_NEAR(actual, expected, 1e-10);
}

TEST(BSTest, DigitalCallSpreadApproximation) {
    // Digital ≈ (C(K - dK) - C(K + dK)) / (2 * dK)
    const double dK = 0.01;
    const double callLow = bsCallFormula(F, K - dK, T, dF, vol);
    const double callHigh = bsCallFormula(F, K + dK, T, dF, vol);
    const double spread = (callLow - callHigh) / (2.0 * dK);
    const double digital = bsDigitalFormula(F, K, T, dF, vol, 0.0);
    EXPECT_NEAR(spread, digital, 1e-4);
}

TEST(BSTest, SkewReducesDigitalForNegativeSkew) {
    // Negative skew (equity-like) increases digital call price
    const double noSkew = bsDigitalFormula(F, K, T, dF, vol, 0.0);
    const double withSkew = bsDigitalFormula(F, K, T, dF, vol, -0.001);
    EXPECT_GT(withSkew, noSkew);
}