#include "pricer/BSPricer.h"

#include <gtest/gtest.h>

#include "market/SimpleMarket.h"
#include "payoff/PayoffNode.h"
#include "pricer/BSFormula.h"

using namespace market;
using namespace payoff;
using namespace pricer;

class BSPricerTest : public ::testing::Test {
   protected:
    const Date pricingDate = makeDate(2025, 1, 15);
    const Date fixingDate = makeDate(2026, 1, 15);
    const Date settlementDate = makeDate(2026, 1, 17);
    const double spot = 100.0;
    const double rate = 0.05;
    const double T = yearFraction(pricingDate, fixingDate);
    const double dF = std::exp(-rate * yearFraction(pricingDate, settlementDate));
    const double forward = spot * std::exp(rate * T);

    // Negative skew, some curvature
    const vol::SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    SimpleMarket market{pricingDate, "SPY", spot, rate, sviParams};

    double volAt(const double K) const {
        const auto slice = market.getBSVolSlice(fixingDate);
        return slice->vol(K);
    }

    double skewAt(const double K) const {
        const auto slice = market.getBSVolSlice(fixingDate);
        return slice->dVolDStrike(K);
    }
};

TEST_F(BSPricerTest, ATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = CashPayment(max(S - K, 0.0), settlementDate);

    const double pricerPrice = bsPrice(payoff, market);
    const double formulaPrice = blackCallFormula(forward, K, T, dF, volAt(K));

    EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
}

TEST_F(BSPricerTest, OTMCall) {
    constexpr double K = 110.0;

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = CashPayment(max(S - K, 0.0), settlementDate);

    const double pricerPrice = bsPrice(payoff, market);
    const double formulaPrice = blackCallFormula(forward, K, T, dF, volAt(K));

    EXPECT_NEAR(pricerPrice, formulaPrice, 1e-10);
}

TEST_F(BSPricerTest, DigitalCall) {
    constexpr double K = 100.0;

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = CashPayment(ite(S >= K, 1.0, 0.0), settlementDate);

    const double pricerPrice = bsPrice(payoff, market);
    const double formulaPrice = blackDigitalFormula(forward, K, T, dF, volAt(K), skewAt(K));

    EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
}

TEST_F(BSPricerTest, PutCallParity) {
    constexpr double K = 100.0;

    const auto S = fixing("SPY", fixingDate);
    const auto call = CashPayment(max(S - K, 0.0), settlementDate);
    const auto put = CashPayment(max(K - S, 0.0), settlementDate);

    const double callPrice = bsPrice(call, market);
    const double putPrice = bsPrice(put, market);

    EXPECT_NEAR(callPrice - putPrice, dF * (forward - K), 1e-10);
}

TEST_F(BSPricerTest, ForwardContract) {
    constexpr double K = 105.0;

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = CashPayment(S - K, settlementDate);

    const double pricerPrice = bsPrice(payoff, market);

    EXPECT_NEAR(pricerPrice, dF * (forward - K), 1e-10);
}