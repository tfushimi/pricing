#include "numerics/interpolation/LinearInterpolator.h"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace numerics::interpolation;

TEST(LinearInterpolatorTest, InterpolatesAtKnots) {
    const LinearInterpolator interp({0.0, 1.0, 2.0}, {0.0, 2.0, 6.0});
    EXPECT_DOUBLE_EQ(interp(0.0), 0.0);
    EXPECT_DOUBLE_EQ(interp(1.0), 2.0);
}

TEST(LinearInterpolatorTest, InterpolatesMidpoint) {
    const LinearInterpolator interp({0.0, 1.0, 2.0}, {0.0, 2.0, 6.0});
    EXPECT_DOUBLE_EQ(interp(0.5), 1.0);  // slope=2 on [0,1)
    EXPECT_DOUBLE_EQ(interp(1.5), 4.0);  // slope=4 on [1,2)
}

TEST(LinearInterpolatorTest, ExtrapolatesLeft) {
    // slope=2 on first segment, so f(-1) = 2*(-1) + 0 = -2
    const LinearInterpolator interp({0.0, 1.0, 2.0}, {0.0, 2.0, 6.0});
    EXPECT_DOUBLE_EQ(interp(0.0), 0.0);  // leftmost knot
    EXPECT_DOUBLE_EQ(interp(-1.0), -2.0);
}

TEST(LinearInterpolatorTest, ExtrapolatesRight) {
    // slope=4 on last segment, so f(3) = 4*3 - 2 = 10
    const LinearInterpolator interp({0.0, 1.0, 2.0}, {0.0, 2.0, 6.0});
    EXPECT_DOUBLE_EQ(interp(2.0), 6.0);  // rightmost knot
    EXPECT_DOUBLE_EQ(interp(3.0), 10.0);
}

TEST(LinearInterpolatorTest, ThrowsOnSizeMismatch) {
    EXPECT_THROW(LinearInterpolator({0.0, 1.0}, {0.0}), std::invalid_argument);
}

TEST(LinearInterpolatorTest, ThrowsOnTooFewPoints) {
    EXPECT_THROW(LinearInterpolator({0.0}, {1.0}), std::invalid_argument);
}

TEST(LinearInterpolatorTest, ThrowsOnUnsortedX) {
    EXPECT_THROW(LinearInterpolator({0.0, 2.0, 1.0}, {0.0, 1.0, 2.0}), std::invalid_argument);
}

TEST(LinearInterpolatorTest, ThrowsOnDuplicateX) {
    EXPECT_THROW(LinearInterpolator({0.0, 1.0, 1.0}, {0.0, 1.0, 2.0}), std::invalid_argument);
}
