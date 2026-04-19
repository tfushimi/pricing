#include "pricer/MCPricerWrapper.h"

#include <gtest/gtest.h>

#include "common/Date.h"
#include "market/SimpleMarket.h"
#include "payoff/Observable.h"
#include "pricer/BSFormula.h"
#include "pricer/HestonFormula.h"

using namespace calendar;
using namespace market;
using namespace payoff;
using namespace pricer;

class MCPricerTest : public ::testing::Test {
   protected:
    const std::string symbol = "SPX";
    const Date pricingDate = makeDate(2025, 1, 15);
    const Date fixingDate = makeDate(2026, 1, 15);
    const Date settlementDate = makeDate(2026, 1, 17);
    const double spot = 100.0;
    const double rate = 0.03;
    const double dividend = 0.02;
    const double T = yearFraction(pricingDate, fixingDate);
    const double dF = std::exp(-rate * yearFraction(pricingDate, settlementDate));

    const HestonParams hestonParams{0.05, 5.0, 0.05, 0.5, -0.8};
    const SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    SimpleMarket market{pricingDate, symbol, spot, rate, dividend, sviParams};

    double volAt(const double K) const { return market.getBSVolSlice(symbol, fixingDate).vol(K); }
};

TEST_F(MCPricerTest, GBMPricerATMCall) {
    constexpr double K = 100.0;
    const auto pf = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);

    const double mcPrice = GBMMCPricer{market, volAt(K), 100'000}.price(pf);
    const double formulaPrice = bsCallFormula(market.getForward(symbol, T), K, T, dF, volAt(K));

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, GBMPricerOTMCall) {
    constexpr double K = 110.0;
    const auto pf = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);

    const double mcPrice = GBMMCPricer{market, volAt(K), 100'000}.price(pf);
    const double formulaPrice = bsCallFormula(market.getForward(symbol, T), K, T, dF, volAt(K));

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, GBMPricerDigitalCall) {
    constexpr double K = 100.0;
    const auto pf = cashPayment(fixing(symbol, fixingDate) > K, settlementDate);

    const double mcPrice = GBMMCPricer{market, volAt(K), 100'000}.price(pf);
    const double vol = volAt(K);
    const double F = market.getForward(symbol, T);
    const double d2 = std::log(F / K) / (vol * std::sqrt(T)) - 0.5 * vol * std::sqrt(T);
    const double formulaPrice = dF * normalCdf(d2);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.01);
}

TEST_F(MCPricerTest, HestonPricerATMCall) {
    constexpr double K = 100.0;
    const auto pf = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);

    const double mcPrice = HestonMCPricer{market, hestonParams, 100'000}.price(pf);
    const double formulaPrice = hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, HestonPricerOTMCall) {
    constexpr double K = 110.0;
    const auto pf = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);

    const double mcPrice = HestonMCPricer{market, hestonParams, 100'000}.price(pf);
    const double formulaPrice = hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, HestonPricerDigitalCall) {
    constexpr double K = 100.0;
    const auto pf = cashPayment(fixing(symbol, fixingDate) > K, settlementDate);

    const double mcPrice = HestonMCPricer{market, hestonParams, 100'000}.price(pf);
    const double formulaPrice = hestonDigitalCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.01);
}

TEST_F(MCPricerTest, ApproxLocalVolPricerATMCall) {
    constexpr double K = 100.0;
    const auto pf = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);

    const double mcPrice = ApproxLocalVolMCPricer{market, hestonParams, 100'000}.price(pf);
    const double formulaPrice = hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, ApproxLocalVolPricerOTMCall) {
    constexpr double K = 110.0;
    const auto pf = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);

    const double mcPrice = ApproxLocalVolMCPricer{market, hestonParams, 100'000}.price(pf);
    const double formulaPrice = hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}

TEST_F(MCPricerTest, ApproxLocalVolPricerDigitalCall) {
    constexpr double K = 100.0;
    const auto pf = cashPayment(fixing(symbol, fixingDate) > K, settlementDate);

    const double mcPrice = ApproxLocalVolMCPricer{market, hestonParams, 100'000}.price(pf);
    const double formulaPrice = hestonDigitalCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.01);
}

TEST_F(MCPricerTest, ScenarioCaching) {
    constexpr double K = 100.0;
    const auto call = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);
    const auto digital = cashPayment(fixing(symbol, fixingDate) > K, settlementDate);

    const HestonMCPricer p{market, hestonParams, 100'000};
    const auto scenarios = p.generateScenarios(call);

    EXPECT_NEAR(p.priceFromScenarios(call, scenarios),
                hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams), 0.1);
    EXPECT_NEAR(p.priceFromScenarios(digital, scenarios),
                hestonDigitalCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams), 0.01);
}

TEST_F(MCPricerTest, ParallelMCPricer) {
    constexpr double K = 100.0;
    const auto pf = cashPayment(max(fixing(symbol, fixingDate) - K, 0.0), settlementDate);

    const double mcPrice = HestonMCPricer{market, hestonParams, 100'000, 1.0 / 12.0, 8}.price(pf);
    const double formulaPrice = hestonCallFormula(market.getForward(symbol, T), K, T, dF, hestonParams);

    EXPECT_NEAR(mcPrice, formulaPrice, 0.1);
}
