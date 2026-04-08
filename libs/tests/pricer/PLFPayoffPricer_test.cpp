#include <gtest/gtest.h>

#include "common/Date.h"
#include "market/SimpleMarket.h"
#include "payoff/Observable.h"
#include "pricer/BSFormula.h"
#include "pricer/BSPricer.h"
#include "pricer/HestonPricer.h"

using namespace calendar;
using namespace market;
using namespace payoff;
using namespace pricer;

class PLFPayoffPricerTest : public ::testing::Test {
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

    // In p.15  of "The Heston Model and Its Extension in MATLAB and C#"
    const HestonParams hestonParams{0.05, 5.0, 0.05, 0.5, -0.8};

    // Negative skew, some curvature
    const vol::SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    SimpleMarket market{pricingDate, symbol, spot, rate, 0.0, sviParams};

    double volAt(const double K, const Date fixingDate) const {
        const auto& slice = market.getBSVolSlice(symbol, fixingDate);
        return slice.vol(K);
    }

    double skewAt(const double K, const Date fixingDate) const {
        const auto& slice = market.getBSVolSlice(symbol, fixingDate);
        return slice.dVolDStrike(K);
    }
};

TEST_F(PLFPayoffPricerTest, ATMCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate1);

    const double pricerPrice = bsPricer(payoff, market);
    const double formulaPrice = bsCallFormula(forward1, K, T1, dF1, volAt(K, fixingDate1));

    EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
}

TEST_F(PLFPayoffPricerTest, OTMCall) {
    constexpr double K = 110.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(max(S - K, 0.0), settlementDate1);

    {
        const double pricerPrice = bsPricer(payoff, market);
        const double formulaPrice = bsCallFormula(forward1, K, T1, dF1, volAt(K, fixingDate1));

        EXPECT_NEAR(pricerPrice, formulaPrice, 1e-10);
    }

    {
        const double pricerPrice = hestonPricer(payoff, market, hestonParams);
        const double formulaPrice = hestonCallFormula(forward1, K, T1, dF1, hestonParams);

        EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
    }
}

TEST_F(PLFPayoffPricerTest, DigitalCall) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(ite(S >= K, 1.0, 0.0), settlementDate1);

    {
        const double pricerPrice = bsPricer(payoff, market);
        const double formulaPrice =
            bsDigitalFormula(forward1, K, T1, dF1, volAt(K, fixingDate1), skewAt(K, fixingDate1));

        EXPECT_DOUBLE_EQ(pricerPrice, formulaPrice);
    }

    {
        const double pricerPrice = hestonPricer(payoff, market, hestonParams);
        const double formulaPrice = hestonDigitalCallFormula(forward1, K, T1, dF1, hestonParams);

        EXPECT_NEAR(pricerPrice, formulaPrice, 1e-10);
    }
}

TEST_F(PLFPayoffPricerTest, PutCallParity) {
    constexpr double K = 100.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto call = cashPayment(max(S - K, 0.0), settlementDate1);
    const auto put = cashPayment(max(K - S, 0.0), settlementDate1);

    {
        const double callPrice = bsPricer(call, market);
        const double putPrice = bsPricer(put, market);

        EXPECT_NEAR(callPrice - putPrice, dF1 * (forward1 - K), 1e-10);
    }

    {
        const double callPrice = hestonPricer(call, market, hestonParams);
        const double putPrice = hestonPricer(put, market, hestonParams);

        EXPECT_NEAR(callPrice - putPrice, dF1 * (forward1 - K), 1e-10);
    }
}

TEST_F(PLFPayoffPricerTest, ForwardContract) {
    constexpr double K = 105.0;

    const auto S = fixing(symbol, fixingDate1);
    const auto payoff = cashPayment(S - K, settlementDate1);

    {
        const double pricerPrice = bsPricer(payoff, market);

        EXPECT_NEAR(pricerPrice, dF1 * (forward1 - K), 1e-10);
    }

    {
        const double pricerPrice = hestonPricer(payoff, market, hestonParams);

        EXPECT_NEAR(pricerPrice, dF1 * (forward1 - K), 1e-10);
    }
}

TEST_F(PLFPayoffPricerTest, CombinedPaymentTwoLegs) {
    constexpr double K1 = 100.0;
    constexpr double K2 = 110.0;

    const auto S1 = fixing(symbol, fixingDate1);
    const auto S2 = fixing(symbol, fixingDate2);
    const auto leg1 = cashPayment(max(S1 - K1, 0.0), settlementDate1);
    const auto leg2 = cashPayment(max(S2 - K2, 0.0), settlementDate2);
    const auto payoff = combinedPayment(leg1, leg2);

    {
        const double pricerPrice = bsPricer(payoff, market);
        const double expected = bsCallFormula(forward1, K1, T1, dF1, volAt(K1, fixingDate1)) +
                                bsCallFormula(forward2, K2, T2, dF2, volAt(K2, fixingDate2));
        EXPECT_NEAR(pricerPrice, expected, 1e-10);
    }

    {
        const double pricerPrice = hestonPricer(payoff, market, hestonParams);
        const double expected = hestonCallFormula(forward1, K1, T1, dF1, hestonParams) +
                                hestonCallFormula(forward2, K2, T2, dF2, hestonParams);
        EXPECT_NEAR(pricerPrice, expected, 1e-10);
    }
}
