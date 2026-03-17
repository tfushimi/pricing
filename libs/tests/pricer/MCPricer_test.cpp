#include "pricer/MCPricer.h"

#include <gtest/gtest.h>
#include <pricer/HestonFormula.h>

#include "market/ConstantForwardCurve.h"
#include "market/SimpleMarket.h"
#include "mc/Process.h"
#include "payoff/Observable.h"
#include "pricer/BSFormula.h"

using namespace market;
using namespace payoff;
using namespace pricer;
using namespace mc;

class MCPricerTest : public ::testing::Test {
   protected:
    const std::string symbol = "SPY";
    const Date pricingDate = makeDate(2025, 1, 15);
    const Date fixingDate = makeDate(2026, 1, 15);
    const Date settlementDate = makeDate(2026, 1, 17);
    const double spot = 100.0;
    const double rate = 0.05;
    const double T = yearFraction(pricingDate, fixingDate);
    const double dF = std::exp(-rate * yearFraction(pricingDate, settlementDate));

    // In p.15  of "The Heston Model and Its Extension in MATLAB and C#"
    const HestonParams hestonParams{0.05, 5.0, 0.05, 0.5, -0.8};

    // Negative skew, some curvature
    const vol::SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    SimpleMarket market{pricingDate, symbol, spot, rate, sviParams};

    double volAt(const double K) const {
        const auto& slice = market.getBSVolSlice(symbol, fixingDate);
        return slice.vol(K);
    }

    double skewAt(const double K) const {
        const auto& slice = market.getBSVolSlice(symbol, fixingDate);
        return slice.dVolDStrike(K);
    }
};

TEST_F(MCPricerTest, GBMPricerATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);
    const auto forward = [&](const double t) { return market.getForward(symbol, t); };

    const GBMProcess gbm{forward, volAt(K)};
    const RNG rng(42);

    MCPricer pricer{market, gbm, 1'000'000, rng};
    const double pricerPrice = pricer.price(payoff);
    const double formulaPrice = bsCallFormula(market.getForward(symbol, T), K, T, dF, volAt(K));

    EXPECT_NEAR(pricerPrice, formulaPrice, 1e-3);
}

TEST_F(MCPricerTest, HestonPricerATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);
    const auto forward = [&](const double t) { return market.getForward(symbol, t); };

    const HestonProcess heston{forward, hestonParams};
    const RNG rng(42);

    MCPricer pricer{market, heston, 1'000'000, rng};
    const double pricerPrice = pricer.price(payoff, 1 / 12.0);
    const double formulaPrice =
        hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(pricerPrice, formulaPrice, 1e-2);
}