#include "market/MarketDB.h"

#include <gtest/gtest.h>

#include "common/Date.h"

using namespace calendar;
using namespace market;

class MarketDBTest : public ::testing::Test {
   public:
    const Date pricingDate = makeDate(2026, 4, 26);
    const MarketDB mkt{pricingDate, "db=test_market.db"};
};

TEST_F(MarketDBTest, Price) {
    const auto price = mkt.getPrice("SPX", makeDate(2026, 4, 24));
    ASSERT_TRUE(price.has_value());
    EXPECT_DOUBLE_EQ(*price, 100.0);

    const auto missing = mkt.getPrice("UNKNOWN", makeDate(2026, 4, 24));
    EXPECT_FALSE(missing.has_value());
}
