#include <gtest/gtest.h>

#include "common/types.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace payoff;

static const Date D1 = makeDate(2026, 1, 15);
static const Date D2 = makeDate(2026, 6, 15);

static Scenario makeScenario(Date date, std::initializer_list<double> values) {
    return {{date, Sample(std::data(values), values.size())}};
}

static Scenario makeScenario2(Date d1, const std::initializer_list<double> v1, Date d2,
                              const std::initializer_list<double> v2) {
    return {
        {d1, Sample(std::data(v1), v1.size())},
        {d2, Sample(std::data(v2), v2.size())},
    };
}

TEST(ApplyFixingsTest, VanillaCall) {
    const auto scenario = makeScenario(D1, {110.0, 100.0, 90.0});
    constexpr double K = 100.0;

    const auto payoff = max(fixing("SPY", D1) - constant(K), constant(0.0));
    const auto result = applyFixings(payoff, scenario);

    EXPECT_DOUBLE_EQ(result[0], 10.0);
    EXPECT_DOUBLE_EQ(result[1], 0.0);
    EXPECT_DOUBLE_EQ(result[2], 0.0);
}

TEST(ApplyFixingsTest, VanillaPut) {
    const auto scenario = makeScenario(D1, {110.0, 100.0, 90.0});
    constexpr double K = 100.0;

    const auto payoff = max(constant(K) - fixing("SPY", D1), constant(0.0));
    const auto result = applyFixings(payoff, scenario);

    EXPECT_DOUBLE_EQ(result[0], 0.0);
    EXPECT_DOUBLE_EQ(result[1], 0.0);
    EXPECT_DOUBLE_EQ(result[2], 10.0);
}

TEST(ApplyFixingsTest, BullSpread) {
    const auto scenario = makeScenario(D1, {90.0, 105.0, 120.0});
    constexpr double K1 = 100.0, K2 = 110.0;

    const auto call1 = max(fixing("SPY", D1) - constant(K1), constant(0.0));
    const auto call2 = max(fixing("SPY", D1) - constant(K2), constant(0.0));
    const auto payoff = call1 - call2;
    const auto result = applyFixings(payoff, scenario);

    EXPECT_DOUBLE_EQ(result[0], 0.0);
    EXPECT_DOUBLE_EQ(result[1], 5.0);
    EXPECT_DOUBLE_EQ(result[2], 10.0);
}

TEST(ApplyFixingsTest, AverageCall) {
    const auto scenario = makeScenario2(D1, {120.0, 105.0, 90.0}, D2, {115.0, 108.0, 85.0});
    constexpr double K = 100.0;

    const auto S1 = fixing("SPY", D1);
    const auto S2 = fixing("SPY", D2);
    const auto payoff = max(min(S1, S2) - constant(K), constant(0.0));
    const auto result = applyFixings(payoff, scenario);

    EXPECT_DOUBLE_EQ(result[0], 15.0);
    EXPECT_DOUBLE_EQ(result[1], 5.0);
    EXPECT_DOUBLE_EQ(result[2], 0.0);
}

TEST(ApplyFixingsTest, BarrierEnhancedNote) {
    const auto scenario = makeScenario(D1, {70.0, 90.0, 110.0, 130.0});
    constexpr double barrier = 80.0, notional = 100.0, cap = 120.0;

    const auto S = fixing("SPY", D1);
    const auto participation = constant(notional) + constant(1.1) * (S - constant(notional));
    const auto protectedPayoff = min(max(participation, constant(notional)), constant(cap));
    const auto payoff = ite(S >= constant(barrier), protectedPayoff, S);
    const auto result = applyFixings(payoff, scenario);

    EXPECT_DOUBLE_EQ(result[0], 70.0);
    EXPECT_DOUBLE_EQ(result[1], 100.0);
    EXPECT_DOUBLE_EQ(result[2], 111.0);
    EXPECT_DOUBLE_EQ(result[3], 120.0);
}