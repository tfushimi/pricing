#include "pricer/BSPricer.h"

#include <gtest/gtest.h>

#include "market/SimpleMarket.h"
#include "payoff/Observable.h"
#include "pricer/BSFormula.h"

using namespace market;
using namespace payoff;
using namespace pricer;

class BSPricerTest : public ::testing::Test {
   protected:
    const std::string symbol{"SPY"};
    const Date pricingDate = makeDate(2025, 1, 15);
    const Date fixingDate1 = makeDate(2026, 1, 15);
    const Date settlementDate1 = makeDate(2026, 1, 17);
    const Date fixingDate2 = makeDate(2026, 4, 15);
    const Date settlementDate2 = makeDate(2026, 4, 17);

    const double spot = 100.0;
    const double rate = 0.05;
    const double T1 = yearFraction(pricingDate, fixingDate1);
    const double T2 = yearFraction(pricingDate, fixingDate2);
    const double dF1 = std::exp(-rate * yearFraction(pricingDate, settlementDate1));
    const double dF2 = std::exp(-rate * yearFraction(pricingDate, settlementDate2));
    const double forward1 = spot * std::exp(rate * T1);
    const double forward2 = spot * std::exp(rate * T2);

    // Negative skew, some curvature
    const vol::SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    SimpleMarket market{pricingDate, symbol, spot, rate, sviParams};

    double volAt(const double K, const Date fixingDate) const {
        const auto& slice = market.getBSVolSlice(symbol, fixingDate);
        return slice.vol(K);
    }

    double skewAt(const double K, const Date fixingDate) const {
        const auto& slice = market.getBSVolSlice(symbol, fixingDate);
        return slice.dVolDStrike(K);
    }

    void SetUp() override {
        market.getOrCreateBSVolSlice(symbol, fixingDate1);
        market.getOrCreateBSVolSlice(symbol, fixingDate2);
    }
};

TEST_F(BSPricerTest, ATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate1);

    const double pricerPrice = bsPricer(payoff, market);
    const double formulaPrice = bsCallFormula(forward1, K, T1, dF1, volAt(K, fixingDate1));

    EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
}

TEST_F(BSPricerTest, OTMCall) {
    constexpr double K = 110.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate1);

    const double pricerPrice = bsPricer(payoff, market);
    const double formulaPrice = bsCallFormula(forward1, K, T1, dF1, volAt(K, fixingDate1));

    EXPECT_NEAR(pricerPrice, formulaPrice, 1e-10);
}

TEST_F(BSPricerTest, DigitalCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(ite(S >= K, 1.0, 0.0), settlementDate1);

    const double pricerPrice = bsPricer(payoff, market);
    const double formulaPrice =
        bsDigitalFormula(forward1, K, T1, dF1, volAt(K, fixingDate1), skewAt(K, fixingDate1));

    EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
}

TEST_F(BSPricerTest, PutCallParity) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto call = cashPayment(max(S - K, 0.0), settlementDate1);
    const auto put = cashPayment(max(K - S, 0.0), settlementDate1);

    const double callPrice = bsPricer(call, market);
    const double putPrice = bsPricer(put, market);

    EXPECT_NEAR(callPrice - putPrice, dF1 * (forward1 - K), 1e-10);
}

TEST_F(BSPricerTest, ForwardContract) {
    constexpr double K = 105.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(S - K, settlementDate1);

    const double pricerPrice = bsPricer(payoff, market);

    EXPECT_NEAR(pricerPrice, dF1 * (forward1 - K), 1e-10);
}

TEST_F(BSPricerTest, CombinedPaymentTwoLegs) {
    constexpr double K1 = 100.0;
    constexpr double K2 = 110.0;

    const auto S1 = fixing(symbol, fixingDate1);
    const auto S2 = fixing(symbol, fixingDate2);
    const auto leg1 = cashPayment(max(S1 - K1, 0.0), settlementDate1);
    const auto leg2 = cashPayment(max(S2 - K2, 0.0), settlementDate2);
    const auto payoff = combinedPayment(leg1, leg2);

    const double pricerPrice = bsPricer(payoff, market);
    const double expected = bsCallFormula(forward1, K1, T1, dF1, volAt(K1, fixingDate1)) +
                            bsCallFormula(forward2, K2, T2, dF2, volAt(K2, fixingDate2));
    EXPECT_NEAR(pricerPrice, expected, 1e-10);
}

TEST_F(BSPricerTest, MultiplyPayment) {
    constexpr double K = 100.0;
    constexpr double multiplier = 2.5;

    const auto S = fixing(symbol, fixingDate1);
    const auto payment = cashPayment(max(S - K, 0.0), settlementDate1);
    const auto payoff = multiplyPayment(payment, multiplier);

    const double pricerPrice = bsPricer(payoff, market);
    const double expected = multiplier * bsCallFormula(forward1, K, T1, dF1, volAt(K, fixingDate1));

    EXPECT_NEAR(pricerPrice, expected, 1e-10);
}

TEST_F(BSPricerTest, CombinedAndMultiPaymentNested) {
    constexpr double K = 100.0;
    constexpr double cap = 120.0;
    constexpr double notional = 1.1;

    const auto S = fixing(symbol, fixingDate1);
    const auto longLeg = multiplyPayment(cashPayment(max(S - K, 0.0), settlementDate1), notional);
    const auto shortLeg =
        multiplyPayment(cashPayment(max(S - cap, 0.0), settlementDate1), -notional);
    const auto payoff = combinedPayment(longLeg, shortLeg);

    const double pricerPrice = bsPricer(payoff, market);
    const double expected =
        notional * bsCallFormula(forward1, K, T1, dF1, volAt(K, fixingDate1)) -
        notional * bsCallFormula(forward1, cap, T1, dF1, volAt(cap, fixingDate1));

    EXPECT_NEAR(pricerPrice, expected, 1e-10);
}