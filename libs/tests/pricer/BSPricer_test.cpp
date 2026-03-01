#include "pricer/BSPricer.h"

#include <gtest/gtest.h>

#include "market/SimpleMarket.h"
#include "payoff/PayoffNode.h"
#include "pricer/BSFormula.h"

using namespace payoff;
using namespace pricer;
using namespace market;
using namespace bs;

class BSPricerTest : public ::testing::Test {
   protected:
    const Date pricingDate = makeDate(2025, 1, 15);
    const Date maturity = makeDate(2026, 1, 15);
    const double spot = 100.0;
    const double rate = 0.05;
    const double T = yearFraction(pricingDate, maturity);
    const double dF = std::exp(-rate * T);
    const double forward = spot / dF;

    // Negative skew, some curvature
    const vol::SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    SimpleMarket market{pricingDate, "SPY", spot, rate, sviParams};

    double volAt(const double K) const {
        const auto slice = market.getBSVolSlice(maturity);
        return slice->vol(K);
    }

    double skewAt(const double K) const {
        const auto slice = market.getBSVolSlice(maturity);
        return slice->dVolDStrike(K);
    }
};

TEST_F(BSPricerTest, ATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing("SPY", maturity);
    const auto payoff = max(S - K, 0.0);

    const double pricerPrice = bsPrice(payoff, market);
    const double formulaPrice = blackCallFormula(forward, K, T, dF, volAt(K));

    EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
}