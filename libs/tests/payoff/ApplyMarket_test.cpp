#include <gtest/gtest.h>

#include <optional>
#include <unordered_map>

#include "ObservableTestUtils.h"
#include "common/Date.h"
#include "market/Market.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace calendar;
using namespace market;
using namespace payoff;

class MockMarket final : public Market {
   public:
    MockMarket(const Date pricingDate, std::unordered_map<Date, double> priceMap)
        : Market(pricingDate), _priceMap(std::move(priceMap)) {}

    MockMarket(const Date pricingDate, Date fixingDate, double price)
        : Market(pricingDate), _priceMap({{fixingDate, price}}) {}

    std::optional<double> getPrice(const std::string&, const Date& date) const override {
        // symbol ignored in mock — price is keyed by date only
        const auto it = _priceMap.find(date);
        if (it != _priceMap.end()) {
            return it->second;
        }
        return std::nullopt;  // not observed — Fixing node kept as-is
    }

    double getDiscountFactor(double) const override { return 0.0; }

    double getForward(const std::string&, const double) const override { return 0.0; }

    const BSVolSlice& getBSVolSlice(const std::string&, const Date&) const override {
        throw new std::runtime_error("getBSVolSlice not implemented");
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

    const auto S = fixing("SPX", fixingDate);
    const auto payoff = max(S - constant(100.0), constant(0.0));
    const auto result = applyMarket(payoff, market);

    const auto* c = asNode<Constant>(result);
    ASSERT_NE(c, nullptr);
    EXPECT_DOUBLE_EQ(c->getValue(), 5.0);
}

TEST(ApplyMarketTest, KeepsUnobservedFixing) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate = makeDate(2026, 3, 20);

    // Empty market — no prices observed
    const MockMarket market(pricingDate, {});

    const auto S = fixing("SPX", fixingDate);
    const auto result = applyMarket(S, market);

    // Should remain as Fixing
    const auto* f = asNode<Fixing>(result);
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

    const auto S1 = fixing("SPX", date1);
    const auto S2 = fixing("SPX", date2);

    const auto payoff = S1 + S2;
    const auto result = foldConstants(applyMarket(payoff, market));

    // S1 replaced with 105, S2 kept as Fixing
    // Add(Constant(105), Fixing) cannot be folded further
    const auto* addPtr = asNode<Add>(result);
    EXPECT_NE(addPtr, nullptr);
}

TEST(ApplyMarketTest, VectorNodes) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto date1 = makeDate(2026, 3, 20);
    const auto date2 = makeDate(2026, 6, 20);

    const MockMarket market(pricingDate, {
                                             {date1, 105.0},  // observed
                                             // date2 not observed
                                         });

    const auto S1 = fixing("SPX", date1);
    const auto S2 = fixing("SPX", date2);

    {
        const auto payoff = sum(S1, S2);
        const auto result = foldConstants(applyMarket(payoff, market));

        // S1 replaced with 105, S2 kept as Fixing
        // Sum(Constant(105), Fixing) cannot be folded further
        const auto* sumPtr = asNode<Sum>(result);
        EXPECT_NE(sumPtr, nullptr);
        EXPECT_EQ(sumPtr->size(), 2);

        const auto* fixing = asNode<Fixing>(*sumPtr->begin());
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPX");
        EXPECT_EQ(fixing->getDate(), date2);

        const auto* folded = asNode<Constant>(*std::next(sumPtr->begin()));
        ASSERT_NE(folded, nullptr);
        EXPECT_EQ(folded->getValue(), 105.0);
    }

    {
        const auto payoff = min(S1, S2);
        const auto result = foldConstants(applyMarket(payoff, market));

        // S1 replaced with 105, S2 kept as Fixing
        // Min(Constant(105), Fixing) cannot be folded further
        const auto* minPtr = asNode<Min>(result);
        EXPECT_NE(minPtr, nullptr);
        EXPECT_EQ(minPtr->size(), 2);

        const auto* fixing = asNode<Fixing>(*minPtr->begin());
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPX");
        EXPECT_EQ(fixing->getDate(), date2);

        const auto* folded = asNode<Constant>(*std::next(minPtr->begin()));
        ASSERT_NE(folded, nullptr);
        EXPECT_EQ(folded->getValue(), 105.0);
    }

    {
        const auto payoff = max(S1, S2);
        const auto result = foldConstants(applyMarket(payoff, market));

        // S1 replaced with 105, S2 kept as Fixing
        // Max(Constant(105), Fixing) cannot be folded further
        const auto* maxPtr = asNode<Max>(result);
        EXPECT_NE(maxPtr, nullptr);
        EXPECT_EQ(maxPtr->size(), 2);

        const auto* fixing = asNode<Fixing>(*maxPtr->begin());
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPX");
        EXPECT_EQ(fixing->getDate(), date2);

        const auto* folded = asNode<Constant>(*std::next(maxPtr->begin()));
        ASSERT_NE(folded, nullptr);
        EXPECT_EQ(folded->getValue(), 105.0);
    }
}

TEST(ApplyMarketTest, CashPaymentReplaceObservedFixing) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate = makeDate(2026, 3, 20);
    const auto settlementDate = makeDate(2026, 3, 22);

    // Single fixing
    const MockMarket market(pricingDate, fixingDate, 105.0);

    const auto S = fixing("SPX", fixingDate);
    const auto payoff = cashPayment(max(S - constant(100.0), constant(0.0)), settlementDate);
    const auto result = applyMarket(payoff, market);

    const auto* cash = asNode<CashPayment>(result);
    ASSERT_NE(cash, nullptr);
    EXPECT_EQ(cash->getSettlementDate(), settlementDate);

    const auto* c = asNode<Constant>(cash->getAmount());
    ASSERT_NE(c, nullptr);
    EXPECT_DOUBLE_EQ(c->getValue(), 5.0);
}

TEST(ApplyMarketTest, CashPaymentKeepUnobservedFixing) {
    const auto pricingDate = makeDate(2026, 1, 15);
    const auto fixingDate = makeDate(2026, 3, 20);
    const auto settlementDate = makeDate(2026, 3, 22);

    // Empty market — no prices observed
    const MockMarket market(pricingDate, {});

    const auto S = fixing("SPX", fixingDate);
    const auto payoff = cashPayment(S, settlementDate);
    const auto result = applyMarket(payoff, market);

    const auto* cash = asNode<CashPayment>(result);
    ASSERT_NE(cash, nullptr);
    EXPECT_EQ(cash->getSettlementDate(), settlementDate);

    // Should remain as Fixing
    const auto* f = asNode<Fixing>(cash->getAmount());
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

    const auto S1 = fixing("SPX", fixingDate1);
    const auto S2 = fixing("SPX", fixingDate2);
    const auto leg1 = cashPayment(max(S1 - constant(100.0), constant(0.0)), settlementDate1);
    const auto leg2 = cashPayment(max(S2 - constant(100.0), constant(0.0)), settlementDate2);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = applyMarket(payoff, market);

    const auto* combined = asNode<CombinedPayment>(result);
    ASSERT_NE(combined, nullptr);

    const auto* cash1 = asNode<CashPayment>(combined->getLeft());
    ASSERT_NE(cash1, nullptr);
    const auto* c1 = asNode<Constant>(cash1->getAmount());
    ASSERT_NE(c1, nullptr);
    EXPECT_DOUBLE_EQ(c1->getValue(), 5.0);

    const auto* cash2 = asNode<CashPayment>(combined->getRight());
    ASSERT_NE(cash2, nullptr);
    const auto* c2 = asNode<Constant>(cash2->getAmount());
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

    const auto S1 = fixing("SPX", fixingDate1);
    const auto S2 = fixing("SPX", fixingDate2);
    const auto leg1 = cashPayment(max(S1 - constant(100.0), constant(0.0)), settlementDate1);
    const auto leg2 = cashPayment(max(S2 - constant(100.0), constant(0.0)), settlementDate2);
    const auto payoff = combinedPayment(leg1, leg2);
    const auto result = applyMarket(payoff, market);

    const auto* combined = asNode<CombinedPayment>(result);
    ASSERT_NE(combined, nullptr);

    // First leg fully folded
    const auto* cash1 = asNode<CashPayment>(combined->getLeft());
    ASSERT_NE(cash1, nullptr);
    const auto* c1 = asNode<Constant>(cash1->getAmount());
    ASSERT_NE(c1, nullptr);
    EXPECT_DOUBLE_EQ(c1->getValue(), 5.0);

    // Second leg still has a Fixing node
    const auto* cash2 = asNode<CashPayment>(combined->getRight());
    ASSERT_NE(cash2, nullptr);
    const auto* c2 = asNode<Constant>(cash2->getAmount());
    EXPECT_EQ(c2, nullptr);
}