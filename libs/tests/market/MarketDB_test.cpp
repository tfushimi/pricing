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
    EXPECT_NEAR(mkt.getDiscountFactor(0.5), std::exp(-0.05 * 0.5), 1e-6);
    EXPECT_NEAR(mkt.getDiscountFactor(1.0), std::exp(-0.05 * 1.0), 1e-6);

    // monotonically decreasing
    EXPECT_GT(mkt.getDiscountFactor(0.25), mkt.getDiscountFactor(0.5));
    EXPECT_GT(mkt.getDiscountFactor(0.5), mkt.getDiscountFactor(1.0));

    EXPECT_THROW(mkt.getDiscountFactor(-1.0), std::invalid_argument);
}

TEST_F(MarketDBTest, ForwardPrice) {
    EXPECT_DOUBLE_EQ(mkt.getForward("SPX", 0.0), 100.0);

    // log-linear interpolation recovers S*exp((r-q)*T) exactly since seed data uses constant carry
    EXPECT_NEAR(mkt.getForward("SPX", 0.25), 100.0 * std::exp(0.03 * 0.25), 1e-6);
    EXPECT_NEAR(mkt.getForward("SPX", 0.5), 100.0 * std::exp(0.03 * 0.5), 1e-6);
    EXPECT_NEAR(mkt.getForward("SPX", 1.0), 100.0 * std::exp(0.03 * 1.0), 1e-6);

    // monotonically increasing (positive carry: r > q)
    EXPECT_LT(mkt.getForward("SPX", 0.25), mkt.getForward("SPX", 0.5));
    EXPECT_LT(mkt.getForward("SPX", 0.5), mkt.getForward("SPX", 1.0));

    EXPECT_THROW(mkt.getForward("UNKNOWN", 1.0), std::runtime_error);
}

// For a date present in the DB, the slice returns positive vols at all strikes.
TEST_F(MarketDBTest, BSVolSlice_ExactDate_VolIsPositive) {
    const BSVolSlice& slice = mkt.getBSVolSlice("SPX", makeDate(2026, 5, 24));
    for (const double K : {80.0, 90.0, 100.0, 110.0, 120.0}) {
        EXPECT_GT(slice.vol(K), 0.0) << "K=" << K;
    }
}

// The same date always returns the same slice object (no duplicate construction).
TEST_F(MarketDBTest, BSVolSlice_Caching) {
    const BSVolSlice& s1 = mkt.getBSVolSlice("SPX", makeDate(2026, 5, 24));
    const BSVolSlice& s2 = mkt.getBSVolSlice("SPX", makeDate(2026, 5, 24));
    EXPECT_EQ(&s1, &s2);
}

// For a date strictly between two DB expiries the slice interpolates linearly
// in total variance: w(K,T) = w1(K) + alpha*(w2(K)-w1(K)).
TEST_F(MarketDBTest, BSVolSlice_TemporalInterpolation) {
    const BSVolSlice& nearSlice = mkt.getBSVolSlice("SPX", makeDate(2026, 5, 24));
    const BSVolSlice& farSlice = mkt.getBSVolSlice("SPX", makeDate(2026, 7, 24));
    const BSVolSlice& midSlice = mkt.getBSVolSlice("SPX", makeDate(2026, 6, 24));

    const double T1 = nearSlice.time();
    const double T2 = farSlice.time();
    const double T = midSlice.time();
    const double alpha = (T - T1) / (T2 - T1);

    for (const double K : {85.0, 95.0, 100.0, 105.0, 115.0}) {
        const double w1 = nearSlice.vol(K) * nearSlice.vol(K) * T1;
        const double w2 = farSlice.vol(K)  * farSlice.vol(K)  * T2;
        const double wExpected = w1 + alpha * (w2 - w1);
        EXPECT_NEAR(midSlice.vol(K) * midSlice.vol(K) * T, wExpected, 1e-10) << "K=" << K;
    }
}

// Beyond the last DB expiry, the last slice is returned unchanged (flat extrapolation).
TEST_F(MarketDBTest, BSVolSlice_ExtrapolationBeyondLastDate) {
    const BSVolSlice& lastSlice = mkt.getBSVolSlice("SPX", makeDate(2027, 4, 24));
    const BSVolSlice& beyondSlice = mkt.getBSVolSlice("SPX", makeDate(2028, 1, 1));
    for (const double K : {80.0, 100.0, 120.0}) {
        EXPECT_DOUBLE_EQ(beyondSlice.vol(K), lastSlice.vol(K)) << "K=" << K;
    }
}

// Before the first DB expiry, the first slice is returned unchanged (flat extrapolation).
TEST_F(MarketDBTest, BSVolSlice_ExtrapolationBeforeFirstDate) {
    const BSVolSlice& firstSlice = mkt.getBSVolSlice("SPX", makeDate(2026, 5, 24));
    const BSVolSlice& beforeSlice = mkt.getBSVolSlice("SPX", makeDate(2026, 5, 1));
    for (const double K : {80.0, 100.0, 120.0}) {
        EXPECT_DOUBLE_EQ(beforeSlice.vol(K), firstSlice.vol(K)) << "K=" << K;
    }
}

TEST_F(MarketDBTest, BSVolSlice_ThrowsForUnknownSymbol) {
    EXPECT_THROW(mkt.getBSVolSlice("UNKNOWN", makeDate(2026, 5, 24)), std::runtime_error);
}
