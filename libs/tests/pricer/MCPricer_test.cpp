#include "pricer/MCPricer.h"

#include <gtest/gtest.h>
#include <pricer/HestonFormula.h>

#include "market/SimpleMarket.h"
#include "mc/Process.h"
#include "payoff/Observable.h"
#include "pricer/BSFormula.h"
#include "pricer/LocalVolFormula.h"

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
    const double rate = 0.03;
    const double dividend = 0.02;
    const double T = yearFraction(pricingDate, fixingDate);
    const double dF = std::exp(-rate * yearFraction(pricingDate, settlementDate));
    const std::function<double(double)> forward = [&](const double t) { return market.getForward(symbol, t); };

    // In p.15  of "The Heston Model and Its Extension in MATLAB and C#"
    const HestonParams hestonParams{0.05, 5.0, 0.05, 0.5, -0.8};

    const LocalVolProcess::LocalVolFunction localVolFunction = [&](const Sample& logZ, const double time) {
        return approximateLocalVol(hestonParams, logZ, time);
    };

    // Negative skew, some curvature
    const vol::SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    SimpleMarket market{pricingDate, symbol, spot, rate, dividend, sviParams};

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

    const GBMProcess gbm{forward, volAt(K)};

    MCPricer pricer{market, gbm, 100'000};
    const double mcPrice = pricer.price(payoff);
    const double formulaPrice = bsCallFormula(market.getForward(symbol, T), K, T, dF, volAt(K));

    // SE is approximately F*vol*sqrt(T) / sqrt(N) = 100*0.2*1.0 / sqrt(100'000) = 0.063
    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, GBMPricerOTMCall) {
    constexpr double K = 110.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);

    const GBMProcess gbm{forward, volAt(K)};

    MCPricer pricer{market, gbm, 100'000};
    const double mcPrice = pricer.price(payoff);
    const double formulaPrice = bsCallFormula(market.getForward(symbol, T), K, T, dF, volAt(K));

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, GBMPricerDigitalCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff_digital = cashPayment(S > K, settlementDate);

    const GBMProcess gbm{forward, volAt(K)};

    MCPricer pricer{market, gbm, 100'000};
    const double mcPrice = pricer.price(payoff_digital);

    // GBM is flat vol so digital = dF * N(d2) with volAt(K)
    const double vol = volAt(K);
    const double F = market.getForward(symbol, T);
    const double d2 = std::log(F / K) / (vol * std::sqrt(T)) - 0.5 * vol * std::sqrt(T);
    const double formulaPrice = dF * normalCdf(d2);

    // Digital payoff is >=0 and <= 1 so std(payoff) <= 0.5
    // SE <= 0.5 / sqrt(100'000) = 0.0016
    EXPECT_NEAR(mcPrice, formulaPrice, 0.01);
}

TEST_F(MCPricerTest, HestonPricerATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);

    const HestonProcess heston{forward, hestonParams};

    MCPricer pricer{market, heston, 100'000};
    const double mcPrice = pricer.price(payoff);

    const double formulaPrice =
        hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, HestonPricerOTMCall) {
    constexpr double K = 110.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);

    const HestonProcess heston{forward, hestonParams};

    MCPricer pricer{market, heston, 100'000};
    const double mcPrice = pricer.price(payoff);

    const double formulaPrice =
        hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, HestonPricerDigitalCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff_digital = cashPayment(S > K, settlementDate);

    const HestonProcess heston{forward, hestonParams};

    MCPricer pricer{market, heston, 100'000};
    const double mcPrice = pricer.price(payoff_digital);
    const double formulaPrice =
        hestonDigitalCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.01);
}

TEST_F(MCPricerTest, LocalVolPricerATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);

    const LocalVolProcess localVol{forward, localVolFunction};
    MCPricer pricer{market, localVol, 100'000};

    const double mcPrice = pricer.price(payoff);
    const double formulaPrice =
        hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, LocalVolPricerOTMCall) {
    constexpr double K = 110.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);

    const LocalVolProcess localVol{forward, localVolFunction};
    MCPricer pricer{market, localVol, 100'000};

    const double mcPrice = pricer.price(payoff);
    const double formulaPrice =
        hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, LocalVolPricerDigitalCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff_digital = cashPayment(S > K, settlementDate);

    const LocalVolProcess localVol{forward, localVolFunction};
    MCPricer pricer{market, localVol, 100'000};

    const double mcPrice = pricer.price(payoff_digital);
    const double formulaPrice =
        hestonDigitalCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.01);
}

TEST_F(MCPricerTest, ParallelMCPricer) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate);

    // GBM
    const GBMProcess gbm{forward, volAt(K)};
    MCPricer gbmPricer{market, gbm, 100'000, 1 / 12.0, 8};
    const double gbmPrice = gbmPricer.price(payoff);
    const double bsFormulaPrice = bsCallFormula(market.getForward(symbol, T), K, T, dF, volAt(K));

    // SE is approximately F*vol*sqrt(T) / sqrt(N) = 100*0.2*1.0 / sqrt(100'000) = 0.063
    EXPECT_NEAR(gbmPrice, bsFormulaPrice, 0.1);

    // Heston
    const HestonProcess heston{forward, hestonParams};
    MCPricer hestonPricer{market, heston, 100'000, 1 / 12.0, 8};
    const double hestonPrice = hestonPricer.price(payoff);

    const double hestonFormulaPrice =
        hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(hestonPrice, hestonFormulaPrice, 0.1);
}
