#include <gtest/gtest.h>

#include <optional>
#include <unordered_map>

#include "ObservableTestUtils.h"
#include "market/Market.h"
#include "payoff/Observable.h"
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

    std::optional<double> getPrice(const std::string&, const Date& date) const override {
        // symbol ignored in mock — price is keyed by date only
        const auto it = _priceMap.find(date);
        if (it != _priceMap.end())
            return it->second;
        return std::nullopt;  // not observed — Fixing node kept as-is
    }

    std::shared_ptr<Curve> getDiscountCurve() const override { return nullptr; }

    std::shared_ptr<Curve> getForwardCurve(const std::string&) const override { return nullptr; }

    std::shared_ptr<BSVolSlice> getBSVolSlice(const std::string&, const Date&) const override {
        return nullptr;
    }

   private:
    Date _pricingDate;
    std::unordered_map<Date, double> _priceMap;
};

TEST(ApplyMarketTest, ReplaceObservedFixing) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate = makeDate(2026, 3, 20);

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
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate = makeDate(2026, 3, 20);

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
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto date1 = makeDate(2026, 3, 20);
    const auto date2 = makeDate(2026, 6, 20);

    const MockMarket market(pricingDate, {
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

TEST(ApplyMarketTest, CashPaymentReplaceObservedFixing) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate = makeDate(2026, 3, 20);
    const auto settlementDate = makeDate(2026, 3, 22);

    // Single fixing
    const MockMarket market(pricingDate, fixingDate, 105.0);

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = cashPayment(max(S - constant(100.0), constant(0.0)), settlementDate);
    const auto result = applyMarket(payoff, market);

    const auto* cash = asCashPayment(result);
    ASSERT_NE(cash, nullptr);
    EXPECT_EQ(cash->getSettlementDate(), settlementDate);

    const auto* c = asConstant(cash->getAmountPtr());
    ASSERT_NE(c, nullptr);
    EXPECT_DOUBLE_EQ(c->getValue(), 5.0);
}

TEST(ApplyMarketTest, CashPaymentKeepUnobservedFixing) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate = makeDate(2026, 3, 20);
    const auto settlementDate = makeDate(2026, 3, 22);

    // Empty market — no prices observed
    const MockMarket market(pricingDate, {});

    const auto S = fixing("SPY", fixingDate);
    const auto payoff = cashPayment(S, settlementDate);
    const auto result = applyMarket(payoff, market);

    const auto* cash = asCashPayment(result);
    ASSERT_NE(cash, nullptr);
    EXPECT_EQ(cash->getSettlementDate(), settlementDate);

    // Should remain as Fixing
    const auto* f = asFixing(cash->getAmountPtr());
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->getDate(), fixingDate);
}

TEST(ApplyMarketTest, CombinedPaymentReplacesBothLegs) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate1 = makeDate(2026, 3, 20);
    const auto fixingDate2 = makeDate(2026, 6, 20);
    const auto settlementDate1 = makeDate(2026, 3, 22);
    const auto settlementDate2 = makeDate(2026, 6, 22);

    const MockMarket market(pricingDate, {{fixingDate1, 105.0}, {fixingDate2, 110.0}});

    const auto S1 = fixing("SPY", fixingDate1);
    const auto S2 = fixing("SPY", fixingDate2);
    const auto leg1 = cashPayment(max(S1 - constant(100.0), constant(0.0)), settlementDate1);
    const auto leg2 = cashPayment(max(S2 - constant(100.0), constant(0.0)), settlementDate2);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = applyMarket(payoff, market);

    const auto* combined = asCombinedPayment(result);
    ASSERT_NE(combined, nullptr);

    const auto* cash1 = asCashPayment(combined->getLeftPtr());
    ASSERT_NE(cash1, nullptr);
    const auto* c1 = asConstant(cash1->getAmountPtr());
    ASSERT_NE(c1, nullptr);
    EXPECT_DOUBLE_EQ(c1->getValue(), 5.0);

    const auto* cash2 = asCashPayment(combined->getRightPtr());
    ASSERT_NE(cash2, nullptr);
    const auto* c2 = asConstant(cash2->getAmountPtr());
    ASSERT_NE(c2, nullptr);
    EXPECT_DOUBLE_EQ(c2->getValue(), 10.0);
}

TEST(ApplyMarketTest, CombinedPaymentPartiallyObserved) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate1 = makeDate(2026, 3, 20);
    const auto fixingDate2 = makeDate(2026, 6, 20);
    const auto settlementDate1 = makeDate(2026, 3, 22);
    const auto settlementDate2 = makeDate(2026, 6, 22);

    // Only first leg observed
    const MockMarket market(pricingDate, {{fixingDate1, 105.0}});

    const auto S1 = fixing("SPY", fixingDate1);
    const auto S2 = fixing("SPY", fixingDate2);
    const auto leg1 = cashPayment(max(S1 - constant(100.0), constant(0.0)), settlementDate1);
    const auto leg2 = cashPayment(max(S2 - constant(100.0), constant(0.0)), settlementDate2);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = applyMarket(payoff, market);

    const auto* combined = asCombinedPayment(result);
    ASSERT_NE(combined, nullptr);

    // First leg fully folded
    const auto* cash1 = asCashPayment(combined->getLeftPtr());
    ASSERT_NE(cash1, nullptr);
    const auto* c1 = asConstant(cash1->getAmountPtr());
    ASSERT_NE(c1, nullptr);
    EXPECT_DOUBLE_EQ(c1->getValue(), 5.0);

    // Second leg still has a Fixing node
    const auto* cash2 = asCashPayment(combined->getRightPtr());
    ASSERT_NE(cash2, nullptr);
    const auto* c2 = asConstant(cash2->getAmountPtr());
    EXPECT_EQ(c2, nullptr);
}