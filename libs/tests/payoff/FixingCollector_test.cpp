#include <gtest/gtest.h>

#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace payoff;

TEST(FixingCollectorTest, SingleFixing) {
    const Date fixingDate = makeDate(2026, 3, 20);

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = max(S - 100.0, 0.0);
    const auto result = getFixings(payoff);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.begin()->getSymbol(), "SPY");
    EXPECT_EQ(result.begin()->getDate(), fixingDate);
}

TEST(FixingCollectorTest, DuplicateFixing) {
    const Date fixingDate = makeDate(2026, 3, 20);

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = max((S + S) / 2 - 100.0, 0.0);
    const auto result = getFixings(payoff);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.begin()->getSymbol(), "SPY");
    EXPECT_EQ(result.begin()->getDate(), fixingDate);
}

TEST(FixingCollectorTest, MultipleDates) {
    const Date fixingDate1 = makeDate(2026, 3, 20);
    const Date fixingDate2 = makeDate(2027, 3, 20);

    const auto S1 = fixing("SPY", fixingDate1);
    const auto S2 = fixing("SPY", fixingDate2);
    const auto payoff = max((S1 + S2) / 2 - 100.0, 0.0);
    const auto result = getFixings(payoff);

    EXPECT_EQ(result.size(), 2);

    auto it = result.begin();
    EXPECT_EQ(it->getSymbol(), "SPY");
    EXPECT_EQ(it->getDate(), fixingDate1);

    it = std::next(it, 1);
    EXPECT_EQ(it->getSymbol(), "SPY");
    EXPECT_EQ(it->getDate(), fixingDate2);
}

TEST(FixingCollectorTest, MultipleSymbols) {
    const Date fixingDate = makeDate(2026, 3, 20);

    const auto S1 = fixing("SPY", fixingDate);
    const auto S2 = fixing("QQQ", fixingDate);
    const auto payoff = max((S1 + S2) / 2 - 100.0, 0.0);
    const auto result = getFixings(payoff);

    EXPECT_EQ(result.size(), 2);

    auto it = result.begin();
    EXPECT_EQ(it->getSymbol(), "QQQ");
    EXPECT_EQ(it->getDate(), fixingDate);

    it = std::next(it, 1);
    EXPECT_EQ(it->getSymbol(), "SPY");
    EXPECT_EQ(it->getDate(), fixingDate);
}

TEST(FixingCollectorTest, CashPayment) {
    const Date fixingDate = makeDate(2026, 3, 20);
    const Date settlementDate = makeDate(2026, 3, 22);

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = cashPayment(max(S - 100.0, 0.0), settlementDate);

    const auto result = getFixings(payoff);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.begin()->getSymbol(), "SPY");
    EXPECT_EQ(result.begin()->getDate(), fixingDate);
}

TEST(FixingCollectorTest, CombinedPayment) {
    const auto fixingDate1 = makeDate(2026, 3, 20);
    const auto settlementDate1 = makeDate(2026, 3, 22);
    const auto fixingDate2 = makeDate(2027, 3, 20);
    const auto settlementDate2 = makeDate(2027, 3, 22);

    const auto S1 = fixing("SPY", fixingDate1);
    const auto S2 = fixing("SPY", fixingDate2);
    const auto leg1 = cashPayment(max(S1 - 100.0, 0.0), settlementDate1);
    const auto leg2 = cashPayment(max(S2 - 100.0, 0.0), settlementDate2);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = getFixings(payoff);

    EXPECT_EQ(result.size(), 2);

    auto it = result.begin();
    EXPECT_EQ(it->getSymbol(), "SPY");
    EXPECT_EQ(it->getDate(), fixingDate1);

    it = std::next(it, 1);
    EXPECT_EQ(it->getSymbol(), "SPY");
    EXPECT_EQ(it->getDate(), fixingDate2);
}

TEST(FixingCollectorTest, CombinedPaymentMultipleSymbols) {
    const Date fixingDate = makeDate(2026, 3, 20);
    const Date settlementDate = makeDate(2026, 3, 22);

    const auto S1 = fixing("SPY", fixingDate);
    const auto S2 = fixing("QQQ", fixingDate);
    const auto leg1 = cashPayment(max(S1 - 100.0, 0.0), settlementDate);
    const auto leg2 = cashPayment(max(S2 - 100.0, 0.0), settlementDate);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = getFixings(payoff);

    EXPECT_EQ(result.size(), 2);

    auto it = result.begin();
    EXPECT_EQ(it->getSymbol(), "QQQ");
    it = std::next(it, 1);
    EXPECT_EQ(it->getSymbol(), "SPY");
}