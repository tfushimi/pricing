#include "market/BSVolSlice.h"

#include <gtest/gtest.h>

#include <cmath>

#include "common/Date.h"

using namespace market;
using namespace calendar;

constexpr double forward = 100.0;
const std::vector strikes = {80.0, 90.0, 100.0, 110.0, 120.0};
const std::vector vols = {0.25, 0.20, 0.18, 0.20, 0.25};

// vol at each input strike must round-trip exactly: w = v^2*T, vol = sqrt(w/T) = v.
TEST(InterpolatedBSVolSliceTest, KnotsReproduced) {
    const InterpolatedBSVolSlice slice(forward, 1.0, strikes, vols);
    EXPECT_DOUBLE_EQ(slice.vol(80.0), 0.25);
    EXPECT_DOUBLE_EQ(slice.vol(90.0), 0.20);
    EXPECT_DOUBLE_EQ(slice.vol(100.0), 0.18);
    EXPECT_DOUBLE_EQ(slice.vol(110.0), 0.20);
    EXPECT_DOUBLE_EQ(slice.vol(120.0), 0.25);
}

// Analytic dVol/dK must match central finite difference to within FD truncation error O(dK^2).
TEST(InterpolatedBSVolSliceTest, dVolDStrikeMatchesFD) {
    const InterpolatedBSVolSlice slice(forward, 1.0, strikes, vols);
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
    const InterpolatedBSVolSlice slice(forward, 1.0, strikes, vols);
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
    EXPECT_THROW(InterpolatedBSVolSlice(0.0, 1.0, strikes, vols), std::invalid_argument);
    EXPECT_THROW(InterpolatedBSVolSlice(-1.0, 1.0, strikes, vols), std::invalid_argument);
}

// If both slices carry the same flat vol, the interpolated vol equals that vol at any T.
TEST(TemporallyInterpolatedBSVolSliceTest, SameVolPreserved) {
    const FlatVolSlice near(forward, 1.0, 0.20);
    const FlatVolSlice far(forward, 3.0, 0.20);
    const TemporallyInterpolatedBSVolSlice slice(forward, 2.0, near, far);
    EXPECT_DOUBLE_EQ(slice.vol(90.0), 0.20);
    EXPECT_DOUBLE_EQ(slice.vol(100.0), 0.20);
    EXPECT_DOUBLE_EQ(slice.vol(110.0), 0.20);
}

// Interpolation is in total-variance space: w = w1 + alpha*(w2-w1), vol = sqrt(w/T).
// near: FlatVol(0.20, T=1.0) -> w1 = 0.04
// far:  FlatVol(0.30, T=2.0) -> w2 = 0.18
// T=1.5, alpha=0.5 -> w = 0.11, vol = sqrt(0.11/1.5)
TEST(TemporallyInterpolatedBSVolSliceTest, VolInterpolationByTotalVariance) {
    const FlatVolSlice near(forward, 1.0, 0.20);
    const FlatVolSlice far(forward, 2.0, 0.30);
    const TemporallyInterpolatedBSVolSlice slice(forward, 1.5, near, far);
    const double expected = std::sqrt(0.11 / 1.5);
    EXPECT_NEAR(slice.vol(90.0), expected, 1e-10);
    EXPECT_NEAR(slice.vol(100.0), expected, 1e-10);
    EXPECT_NEAR(slice.vol(110.0), expected, 1e-10);
}

// Analytic dVol/dK must match central FD. Uses InterpolatedBSVolSlice for a non-trivial smile.
TEST(TemporallyInterpolatedBSVolSliceTest, dVolDStrikeMatchesFD) {
    const InterpolatedBSVolSlice near(forward, 1.0, strikes, {0.25, 0.20, 0.18, 0.20, 0.25});
    const InterpolatedBSVolSlice far(forward, 2.0, strikes, {0.30, 0.25, 0.22, 0.25, 0.30});
    const TemporallyInterpolatedBSVolSlice slice(forward, 1.5, near, far);
    constexpr double dK = 1e-4;
    for (const double K : {85.0, 95.0, 100.0, 105.0, 115.0}) {
        const double analytic = slice.dVolDStrike(K);
        const double fd = (slice.vol(K + dK) - slice.vol(K - dK)) / (2.0 * dK);
        EXPECT_NEAR(analytic, fd, 1e-6) << "K=" << K;
    }
}

TEST(TemporallyInterpolatedBSVolSliceTest, ThrowsOnTimeOutOfRange) {
    const FlatVolSlice near(forward, 1.0, 0.20);
    const FlatVolSlice far(forward, 2.0, 0.20);
    EXPECT_THROW(TemporallyInterpolatedBSVolSlice(forward, 1.0, near, far), std::invalid_argument);
    EXPECT_THROW(TemporallyInterpolatedBSVolSlice(forward, 2.0, near, far), std::invalid_argument);
    EXPECT_THROW(TemporallyInterpolatedBSVolSlice(forward, 0.5, near, far), std::invalid_argument);
    EXPECT_THROW(TemporallyInterpolatedBSVolSlice(forward, 3.0, near, far), std::invalid_argument);
}

TEST(TemporallyInterpolatedBSVolSliceTest, ThrowsOnInvalidSliceOrder) {
    const FlatVolSlice near(forward, 1.0, 0.20);
    const FlatVolSlice far(forward, 2.0, 0.20);
    EXPECT_THROW(TemporallyInterpolatedBSVolSlice(forward, 1.5, far, near), std::invalid_argument);
    EXPECT_THROW(TemporallyInterpolatedBSVolSlice(forward, 1.5, near, near), std::invalid_argument);
}
