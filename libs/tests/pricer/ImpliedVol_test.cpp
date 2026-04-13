#include "pricer/ImpliedVol.h"

#include <gtest/gtest.h>

constexpr double F = 100.0;
constexpr double K = 100.0;
constexpr double T = 1.0;
constexpr double dF = 1.0;  // zero rates

using namespace pricer;

TEST(ImpliedVolTest, ATN) {
    constexpr double vol = 0.20;
    const double price = bsCallFormula(F, K, T, dF, vol);
    EXPECT_NEAR(impliedVol(price, F, K, T, dF), vol, 1e-6);
}

TEST(ImpliedVolTest, OTM) {
    constexpr double vol = 0.30;
    const double price = bsCallFormula(F, 120.0, T, dF, vol);
    EXPECT_NEAR(impliedVol(price, F, 120.0, T, dF), vol, 1e-6);
}

TEST(ImpliedVolTest, ITM) {
    constexpr double vol = 0.15;
    const double price = bsCallFormula(F, 80.0, T, dF, vol);
    EXPECT_NEAR(impliedVol(price, F, 80.0, T, dF), vol, 1e-6);
}

TEST(ImpliedVolTest, RoundtripShortMaturity) {
    constexpr double vol = 0.25;
    const double price = bsCallFormula(F, K, 0.1, dF, vol);
    EXPECT_NEAR(impliedVol(price, F, K, 0.1, dF), vol, 1e-6);
}

TEST(HestonImpliedVolTest, ATMReasonableVol) {
    // Heston-Nandi params: v0=0.04, kappa=10, theta=0.04, xi=1, rho=-1
    // sqrt(v0) = 0.2, so ATM implied vol should be close to 0.2
    constexpr HestonParams params{0.04, 10.0, 0.04, 1.0, -1.0};
    const double vol = hestonImpliedVol(F, K, T, dF, params);
    EXPECT_GT(vol, 0.0);
    EXPECT_NEAR(vol, 0.2, 0.05);
}

TEST(HestonImpliedVolTest, NegativeSkew) {
    // With rho=-1, OTM put (low strike) should have higher implied vol than OTM call (high strike)
    constexpr HestonParams params{0.04, 10.0, 0.04, 1.0, -1.0};
    const double volLow = hestonImpliedVol(F, 80.0, T, dF, params);
    const double volHigh = hestonImpliedVol(F, 120.0, T, dF, params);
    EXPECT_GT(volLow, volHigh);
}
