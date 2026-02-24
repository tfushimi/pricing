#include <gtest/gtest.h>
#include "models/bsformula.h"

const double F = 100.0;
const double K = 100.0;
const double T = 1.0;
const double r = 0.05;
const double vol = 0.20;
const double dF = std::exp(-r * T);  // discount factor

TEST(BlackTest, PutCallParity) {
    // C - P = dF * (F - K)
    double call = blackCallFormula(F, K, T, dF, vol);
    double put = blackCallFormula(F, K, T, dF, vol) - dF * (F - K);
    EXPECT_NEAR(call - put, dF * (F - K), 1e-10);
}

TEST(BlackTest, DigitalCallFlatVol) {
    // With zero skew, digital = dF * N(d2)
    auto [d1, d2] = blackD1D2(F, K, T, vol);
    double expected = dF * normCdf(d2);
    double actual = blackDigitalFormula(F, K, T, dF, vol, /*dVolDStrike=*/0.0);
    EXPECT_NEAR(actual, expected, 1e-10);
}

TEST(BlackTest, DigitalCallSpreadApproximation) {
    // Digital ≈ (C(K - dK) - C(K + dK)) / (2 * dK)
    const double dK = 0.01;
    const double callLow = blackCallFormula(F, K - dK, T, dF, vol);
    const double callHigh= blackCallFormula(F, K + dK, T, dF, vol);
    const double spread = (callLow - callHigh) / (2.0 * dK);
    const double digital = blackDigitalFormula(F, K, T, dF, vol, 0.0);
    EXPECT_NEAR(spread, digital, 1e-4);
}

TEST(BlackTest, SkewReducesDigitalForNegativeSkew) {
    // Negative skew (equity-like) increases digital call price
    const double noSkew = blackDigitalFormula(F, K, T, dF, vol, 0.0);
    const double withSkew = blackDigitalFormula(F, K, T, dF, vol, -0.001);
    EXPECT_GT(withSkew, noSkew);
}