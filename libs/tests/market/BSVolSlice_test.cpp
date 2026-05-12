#include "market/BSVolSlice.h"

#include <gtest/gtest.h>

#include <cmath>

#include "common/Date.h"

using namespace market;
using namespace calendar;

const Date pricingDate = makeDate(2025, 1, 1);
const Date maturityDate = makeDate(2026, 1, 1);
constexpr double forward = 100.0;
const std::vector strikes = {80.0, 90.0, 100.0, 110.0, 120.0};
const std::vector vols = {0.25, 0.20, 0.18, 0.20, 0.25};

// vol at each input strike must round-trip exactly: w = v^2*T, vol = sqrt(w/T) = v.
TEST(InterpolatedBSVolSliceTest, KnotsReproduced) {
    const InterpolatedBSVolSlice slice(pricingDate, forward, maturityDate, strikes, vols);
    EXPECT_DOUBLE_EQ(slice.vol(80.0),  0.25);
    EXPECT_DOUBLE_EQ(slice.vol(90.0),  0.20);
    EXPECT_DOUBLE_EQ(slice.vol(100.0), 0.18);
    EXPECT_DOUBLE_EQ(slice.vol(110.0), 0.20);
    EXPECT_DOUBLE_EQ(slice.vol(120.0), 0.25);
}

// Analytic dVol/dK must match central finite difference to within FD truncation error O(dK^2).
TEST(InterpolatedBSVolSliceTest, dVolDStrikeMatchesFD) {
    const InterpolatedBSVolSlice slice(pricingDate, forward, maturityDate, strikes, vols);
    constexpr double dK = 1e-4;
    for (const double K : {85.0, 95.0, 100.0, 105.0, 115.0}) {
        const double analytic = slice.dVolDStrike(K);
        const double fd = (slice.vol(K + dK) - slice.vol(K - dK)) / (2.0 * dK);
        EXPECT_NEAR(analytic, fd, 1e-6) << "K=" << K;
    }
}

// Beyond the wing strikes, w(k) = vol^2*T must be linear in k = log(K/F):
// three points outside the smile must be collinear in (k, w) space.
TEST(InterpolatedBSVolSliceTest, ExtrapolationLinearInTotalVariance) {
    const InterpolatedBSVolSlice slice(pricingDate, forward, maturityDate, strikes, vols);
    const double T = slice.time();
    auto w = [&](double K) { return slice.vol(K) * slice.vol(K) * T; };
    auto k = [&](double K) { return std::log(K / forward); };

    // Left wing: slope between (k(70),w(70)) and (k(75),w(75)) equals slope at (k(65),w(65)).
    const double leftSlope = (w(75.0) - w(70.0)) / (k(75.0) - k(70.0));
    EXPECT_NEAR((w(65.0) - w(70.0)) / (k(65.0) - k(70.0)), leftSlope, 1e-12);

    // Right wing.
    const double rightSlope = (w(130.0) - w(125.0)) / (k(130.0) - k(125.0));
    EXPECT_NEAR((w(140.0) - w(125.0)) / (k(140.0) - k(125.0)), rightSlope, 1e-12);
}

TEST(InterpolatedBSVolSliceTest, ThrowsOnNonPositiveForward) {
    EXPECT_THROW(InterpolatedBSVolSlice(pricingDate, 0.0,  maturityDate, strikes, vols),
                 std::invalid_argument);
    EXPECT_THROW(InterpolatedBSVolSlice(pricingDate, -1.0, maturityDate, strikes, vols),
                 std::invalid_argument);
}
