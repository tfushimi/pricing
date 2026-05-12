#include "numerics/interpolation/NaturalCubicSplineInterpolator.h"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace numerics::interpolation;

TEST(NaturalCubicSplineInterpolatorTest, InterpolatesAtKnots) {
    const NaturalCubicSplineInterpolator interp({0.0, 1.0, 2.0, 3.0}, {0.0, 1.0, 0.0, 1.0});
    EXPECT_DOUBLE_EQ(interp(0.0), 0.0);
    EXPECT_DOUBLE_EQ(interp(1.0), 1.0);
    EXPECT_DOUBLE_EQ(interp(2.0), 0.0);
    EXPECT_DOUBLE_EQ(interp(3.0), 1.0);
}

// Natural cubic spline on x={0,1,2}, y={0,1,0}:
//   S0(x) = (3/2)x - (1/2)x^3          on (-inf, 1]
//   S1(t) = 1 - (3/2)t^2 + (1/2)t^3   on [1, +inf),  t = x - 1
TEST(NaturalCubicSplineInterpolatorTest, InterpolatesAtMidpoints) {
    const NaturalCubicSplineInterpolator interp({0.0, 1.0, 2.0}, {0.0, 1.0, 0.0});
    EXPECT_DOUBLE_EQ(interp(0.5), 0.6875);
    EXPECT_DOUBLE_EQ(interp(1.5), 0.6875);  // symmetric
}

// For linear data the natural spline is exact (all c[i]=d[i]=0).
TEST(NaturalCubicSplineInterpolatorTest, LinearDataIsExact) {
    const NaturalCubicSplineInterpolator interp({0.0, 1.0, 2.0}, {0.0, 2.0, 4.0});
    EXPECT_DOUBLE_EQ(interp(0.5), 1.0);
    EXPECT_DOUBLE_EQ(interp(1.5), 3.0);
}

// For linear data d[0]=d[n-2]=0 so the boundary segments are also linear,
// giving exact linear extrapolation on both sides.
TEST(NaturalCubicSplineInterpolatorTest, ExtrapolatesLinearlyForLinearData) {
    const NaturalCubicSplineInterpolator interp({0.0, 1.0, 2.0}, {0.0, 2.0, 4.0});
    EXPECT_DOUBLE_EQ(interp(-1.0), -2.0);
    EXPECT_DOUBLE_EQ(interp(3.0), 6.0);
}

// Two-point minimum: single segment, degenerates to a line everywhere.
TEST(NaturalCubicSplineInterpolatorTest, TwoPointsGivesLinearSegment) {
    const NaturalCubicSplineInterpolator interp({0.0, 1.0}, {0.0, 3.0});
    EXPECT_DOUBLE_EQ(interp(0.5), 1.5);
    EXPECT_DOUBLE_EQ(interp(-1.0), -3.0);
    EXPECT_DOUBLE_EQ(interp(2.0), 6.0);
}

TEST(NaturalCubicSplineInterpolatorTest, ThrowsOnSizeMismatch) {
    EXPECT_THROW(NaturalCubicSplineInterpolator({0.0, 1.0}, {0.0}), std::invalid_argument);
}

TEST(NaturalCubicSplineInterpolatorTest, ThrowsOnTooFewPoints) {
    EXPECT_THROW(NaturalCubicSplineInterpolator({0.0}, {1.0}), std::invalid_argument);
}

// --- Vol slice tests ---
// Typical use: implied vol as a function of strike at a fixed expiry.

// 5-point strike/vol smile: knot vols are reproduced exactly.
TEST(NaturalCubicSplineInterpolatorTest, VolSlice_KnotsReproduced) {
    const NaturalCubicSplineInterpolator interp({80.0, 90.0, 100.0, 110.0, 120.0},
                                                {0.25, 0.20, 0.18, 0.20, 0.25});
    EXPECT_DOUBLE_EQ(interp(80.0), 0.25);
    EXPECT_DOUBLE_EQ(interp(90.0), 0.20);
    EXPECT_DOUBLE_EQ(interp(100.0), 0.18);
    EXPECT_DOUBLE_EQ(interp(110.0), 0.20);
    EXPECT_DOUBLE_EQ(interp(120.0), 0.25);
}

// A symmetric smile with equal spacing produces a symmetric spline.
TEST(NaturalCubicSplineInterpolatorTest, VolSlice_SmileSymmetry) {
    const NaturalCubicSplineInterpolator interp({80.0, 90.0, 100.0, 110.0, 120.0},
                                                {0.25, 0.20, 0.18, 0.20, 0.25});
    EXPECT_DOUBLE_EQ(interp(95.0), interp(105.0));
    EXPECT_DOUBLE_EQ(interp(85.0), interp(115.0));
}

// 3-point smile in log-moneyness space: x={-1,0,1}, σ={0.25,0.20,0.25}.
// Spline coefficients (derived analytically):
//   S0(t) = 0.25 - 0.075t + 0.025t^3,  t = x+1,  on (-inf, 0]
//   S1(t) = 0.20 + 0.075t^2 - 0.025t^3, t = x,    on [0, +inf)
// S0(0.5) = S1(0.5) = 0.215625  (equal by symmetry)
TEST(NaturalCubicSplineInterpolatorTest, VolSlice_MidpointInterpolation) {
    const NaturalCubicSplineInterpolator interp({-1.0, 0.0, 1.0}, {0.25, 0.20, 0.25});
    EXPECT_DOUBLE_EQ(interp(-0.5), 0.215625);
    EXPECT_DOUBLE_EQ(interp(0.5), 0.215625);
}

// Beyond the wing strikes the boundary segment's cubic polynomial is continued.
// S0(-0.5) = 0.25 + 0.0375 - 0.003125 = 0.284375
// S1( 1.5) = 0.20 + 0.16875 - 0.084375 = 0.284375
TEST(NaturalCubicSplineInterpolatorTest, VolSlice_ExtrapolationBeyondWings) {
    const NaturalCubicSplineInterpolator interp({-1.0, 0.0, 1.0}, {0.25, 0.20, 0.25});
    EXPECT_DOUBLE_EQ(interp(-1.5), 0.284375);
    EXPECT_DOUBLE_EQ(interp(1.5), 0.284375);
}
