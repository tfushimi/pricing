#include "market/MarketDB.h"

#include <gtest/gtest.h>

#include <cmath>

#include "common/Date.h"

using namespace calendar;
using namespace market;

class MarketDBTest : public ::testing::Test {
   public:
    const Date pricingDate = makeDate(2026, 4, 24);
    const MarketDB mkt{pricingDate, "db=test_market.db"};
};

TEST_F(MarketDBTest, Price) {
    const auto price = mkt.getPrice("SPX", makeDate(2026, 4, 24));
    ASSERT_TRUE(price.has_value());
    EXPECT_DOUBLE_EQ(*price, 100.0);

    const auto missing = mkt.getPrice("UNKNOWN", makeDate(2026, 4, 24));
    EXPECT_FALSE(missing.has_value());
}

TEST_F(MarketDBTest, DiscountFactor) {
    EXPECT_DOUBLE_EQ(mkt.getDiscountFactor(0.0), 1.0);

    // log-linear interpolation recovers exp(-r*T) exactly since seed data uses constant rate
    EXPECT_NEAR(mkt.getDiscountFactor(0.25), std::exp(-0.05 * 0.25), 1e-6);
    EXPECT_NEAR(mkt.getDiscountFactor(0.5),  std::exp(-0.05 * 0.5),  1e-6);
    EXPECT_NEAR(mkt.getDiscountFactor(1.0),  std::exp(-0.05 * 1.0),  1e-6);

    // monotonically decreasing
    EXPECT_GT(mkt.getDiscountFactor(0.25), mkt.getDiscountFactor(0.5));
    EXPECT_GT(mkt.getDiscountFactor(0.5),  mkt.getDiscountFactor(1.0));

    EXPECT_THROW(mkt.getDiscountFactor(-1.0), std::invalid_argument);
}

TEST_F(MarketDBTest, ForwardPrice) {
    EXPECT_DOUBLE_EQ(mkt.getForward("SPX", 0.0), 100.0);

    // log-linear interpolation recovers S*exp((r-q)*T) exactly since seed data uses constant carry
    EXPECT_NEAR(mkt.getForward("SPX", 0.25), 100.0 * std::exp(0.03 * 0.25), 1e-6);
    EXPECT_NEAR(mkt.getForward("SPX", 0.5),  100.0 * std::exp(0.03 * 0.5),  1e-6);
    EXPECT_NEAR(mkt.getForward("SPX", 1.0),  100.0 * std::exp(0.03 * 1.0),  1e-6);

    // monotonically increasing (positive carry: r > q)
    EXPECT_LT(mkt.getForward("SPX", 0.25), mkt.getForward("SPX", 0.5));
    EXPECT_LT(mkt.getForward("SPX", 0.5),  mkt.getForward("SPX", 1.0));

    EXPECT_THROW(mkt.getForward("UNKNOWN", 1.0), std::runtime_error);
}

TEST_F(MarketDBTest, BSVolSlice) {
    const BSVolSlice& volSlice = mkt.getBSVolSlice("SPX", makeDate(2026, 5, 24));

    EXPECT_DOUBLE_EQ(volSlice.vol(100.0), 100.0);
    EXPECT_DOUBLE_EQ(volSlice.vol(120.0), 100.0);

    EXPECT_THROW(mkt.getBSVolSlice("UNKNOWN", makeDate(2026, 5, 24)), std::runtime_error);
}
