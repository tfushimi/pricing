#include <gtest/gtest.h>

#include <optional>
#include <stdexcept>
#include <unordered_map>

#include "PayoffTestUtils.h"
#include "market/Market.h"
#include "payoff/PayoffNode.h"
#include "payoff/Transforms.h"

using namespace market;
using namespace payoff;

class MockMarket final : public Market {
   public:
    MockMarket(const Date pricingDate, std::unordered_map<Date, double> priceMap)
        : _pricingDate(pricingDate), _priceMap(std::move(priceMap)) {}

    MockMarket(const Date pricingDate, Date fixingDate, double price)
        : _pricingDate(pricingDate), _priceMap({{fixingDate, price}}) {}

    Date getPricingDate() const override { return _pricingDate; }

    std::optional<double> getPrice(const std::string& symbol, const Date& date) const override {
        // symbol ignored in mock — price is keyed by date only
        const auto it = _priceMap.find(date);
        if (it != _priceMap.end())
            return it->second;
        return std::nullopt;  // not observed — Fixing node kept as-is
    }

    std::unique_ptr<DiscountCurve> getDiscountFactor(const Date& date) const override {
        return nullptr;
    }

    std::unique_ptr<BSVolSlice> getBSVolSlice(const Date& date) const override { return nullptr; }

   private:
    Date _pricingDate;
    std::unordered_map<Date, double> _priceMap;
};

TEST(ApplyMarketTest, ReplaceObservedFixing) {
    const Date pricingDate = makeDate(2026, 1, 15);
    const Date fixingDate = makeDate(2026, 3, 20);

    // Single fixing
    const MockMarket market(pricingDate, fixingDate, 105.0);

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = max(S - constant(100.0), constant(0.0));
    const auto result = applyMarket(payoff, market);

    const auto* c = asConstant(result);
    ASSERT_NE(c, nullptr);
    EXPECT_DOUBLE_EQ(c->getValue(), 5.0);
}

TEST(ApplyMarketTest, KeepsUnobservedFixing) {
    const Date pricingDate = makeDate(2026, 1, 15);
    const Date fixingDate = makeDate(2026, 3, 20);

    // Empty market — no prices observed
    const MockMarket market(pricingDate, {});

    const auto S = fixing("SPY", fixingDate);
    const auto result = applyMarket(S, market);

    // Should remain as Fixing
    const auto* f = asFixing(result);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->getDate(), fixingDate);
}

TEST(ApplyMarketTest, MultipleFixings) {
    const Date pricingDate = makeDate(2026, 1, 15);
    const Date date1 = makeDate(2026, 3, 20);
    const Date date2 = makeDate(2026, 6, 20);

    MockMarket market(pricingDate, {
                                       {date1, 105.0},  // observed
                                       // date2 not observed
                                   });

    const auto S1 = fixing("SPY", date1);
    const auto S2 = fixing("SPY", date2);
    const auto payoff = S1 + S2;
    const auto result = foldConstants(applyMarket(payoff, market));

    // S1 replaced with 105, S2 kept as Fixing
    // Sum(Constant(105), Fixing) cannot be folded further
    const auto* c = asConstant(result);
    EXPECT_EQ(c, nullptr);  // not fully constant
}