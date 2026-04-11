#include <gtest/gtest.h>

#include "common/Date.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace calendar;
using namespace payoff;
using namespace market;
using namespace mc;

static const Date pricingDate = makeDate(2025, 1, 1);
static const Date D1 = makeDate(2026, 1, 15);
static const Date D2 = makeDate(2026, 6, 15);

class FlatMarket final : public Market {
   public:
    explicit FlatMarket(const double dF) : Market(pricingDate), _dF(dF) {}
    double getDiscountFactor(const double) const override { return _dF; }
    double getForward(const std::string&, const double) const override { return 0.0; }
    std::optional<double> getPrice(const std::string&, const Date&) const override {
        return std::nullopt;
    }
    const BSVolSlice& getBSVolSlice(const std::string&, const Date&) const override {
        throw new std::runtime_error("getBSVolSlice not implemented");
    }

   private:
    double _dF;
};

static Scenario makeScenario(Date date, const std::initializer_list<double> values) {
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

TEST(ApplyFixingsTest, WorstOfCall) {
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

TEST(ApplyFixingsTest, AverageCall) {
    const auto scenario = makeScenario2(D1, {120.0, 105.0, 90.0}, D2, {115.0, 108.0, 85.0});
    constexpr double K = 100.0;

    const auto S1 = fixing("SPY", D1);
    const auto S2 = fixing("SPY", D2);
    const auto payoff = max(sum(S1, S2) / 2.0 - constant(K), constant(0.0));
    const auto result = applyFixings(payoff, scenario);

    EXPECT_DOUBLE_EQ(result[0], 17.5);
    EXPECT_DOUBLE_EQ(result[1], 6.5);
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

TEST(ApplyPayoffFixingsTest, CashPayment) {
    const auto scenario = makeScenario(D1, {110.0, 100.0, 90.0});
    const FlatMarket market(0.95);

    // fixingDate == settlementDate for simplicity
    const auto S = fixing("SPY", D1);
    const auto payoff = cashPayment(max(S - 100.0, 0.0), D1);
    const auto result = applyFixings(payoff, market, scenario);

    EXPECT_DOUBLE_EQ(result[0], 9.5);
    EXPECT_DOUBLE_EQ(result[1], 0.0);
    EXPECT_DOUBLE_EQ(result[2], 0.0);
}

TEST(ApplyPayoffFixingsTest, CombinedPayment) {
    const auto scenario = makeScenario(D1, {110.0, 100.0, 90.0});
    const FlatMarket market(0.95);

    // fixingDate == settlementDate for simplicity
    const auto S = fixing("SPY", D1);
    const auto leg1 = cashPayment(max(S - 100.0, 0.0), D1);
    const auto leg2 = cashPayment(max(100.0 - S, 0.0), D1);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = applyFixings(payoff, market, scenario);

    // call + put = |S - K| discounted
    EXPECT_DOUBLE_EQ(result[0], 9.5);
    EXPECT_DOUBLE_EQ(result[1], 0.0);
    EXPECT_DOUBLE_EQ(result[2], 9.5);
}

TEST(ApplyPayoffFixingsTest, DifferentDiscountFactors) {
    class TwoRateMarket final : public Market {
       public:
        TwoRateMarket() : Market(pricingDate){}
        double getDiscountFactor(const double T) const override {
            return T == yearFraction(pricingDate, D1) ? 0.95 : 0.90;
        }
        double getForward(const std::string&, const double) const override { return 0.0; }
        std::optional<double> getPrice(const std::string&, const Date&) const override {
            return std::nullopt;
        }
        const BSVolSlice& getBSVolSlice(const std::string&, const Date&) const override {
            throw new std::runtime_error("getBSVolSlice not implemented");
        }
    };

    const auto scenario = makeScenario2(D1, {110.0}, D2, {110.0});
    const TwoRateMarket market;

    // fixingDate == settlementDate for simplicity
    const auto S1 = fixing("SPY", D1);
    const auto S2 = fixing("SPY", D2);
    const auto leg1 = cashPayment(max(S1 - 100.0, 0.0), D1);
    const auto leg2 = cashPayment(max(S2 - 100.0, 0.0), D2);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = applyFixings(payoff, market, scenario);

    // leg1: (110-100) * 0.95 = 9.5, leg2: (110 - 100) * 0.9 = 0.9
    EXPECT_DOUBLE_EQ(result[0], 18.5);
}