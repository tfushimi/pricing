#include <gtest/gtest.h>

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "PiecewiseLinearFunction.h"
#include "Segment.h"

using namespace numerics::linear;

static bool near(const double a, const double b, double tol = 1e-10) {
    return std::abs(a - b) < tol;
}

TEST(PwlTest, TestSegment) {
    // f(x) = 2x + 3
    const Segment s(2.0, 3.0, 0.0, 10.0);
    EXPECT_TRUE(near(s(5.0), 13.0)) << "segment eval";
    EXPECT_TRUE(s.contains(0.0)) << "contains lo (inclusive)";
    EXPECT_TRUE(!s.contains(10.0)) << "excludes hi (open)";
    EXPECT_TRUE(!s.containsInterior(0.0)) << "interior excludes lo";
    EXPECT_TRUE(s.containsInterior(5.0)) << "interior contains midpoint";

    // crossing at x=5
    const Segment f(1.0, 0.0, 0.0, 10.0);
    const Segment g(-1.0, 10.0, 0.0, 10.0);  // cross at x=5
    EXPECT_TRUE(near(*f.crossing(g), 5.0)) << "crossing at x=5";

    // no crossing
    const Segment h(1.0, 5.0, 0.0, 10.0);  // parallel to f
    EXPECT_TRUE(!f.crossing(h).has_value()) << "parallel: no crossing";

    // last segment [lo, +inf) contains everything >= lo
    const Segment last(0.0, 1.0, 100.0, POS_INF);
    EXPECT_TRUE(last.contains(1e15)) << "last segment contains large value";

    // invalid construction
    bool threw = false;
    try {
        Segment bad(0.0, 0.0, 10.0, 5.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    EXPECT_TRUE(threw) << "lo >= hi throws";
}

TEST(PwlTest, TestConstruction) {
    // f(x) = 1
    const auto constant = PiecewiseLinearFunction::createConstant(1.0);
    EXPECT_TRUE(near(constant(1e10), 1.0)) << "constant PLF";
    EXPECT_EQ(constant.getBreakPoints().size(), 0) << "constant has no breakpoints";

    // f(x) = 2x + 1
    const auto linear = PiecewiseLinearFunction::createLinear(2.0, 1.0);
    EXPECT_TRUE(near(linear(5.0), 11.0)) << "unbounded linear PLF";

    // bounded: f(x) = x on [10, 20), zero outside
    const auto b = PiecewiseLinearFunction::createLinear(1.0, 0.0, 10.0, 20.0);
    EXPECT_TRUE(near(b(5.0), 0.0)) << "bounded linear: zero left of lo";
    EXPECT_TRUE(near(b(15.0), 15.0)) << "bounded linear: interior";
    EXPECT_TRUE(near(b(20.0), 0.0)) << "bounded linear: zero at hi (open)";
}

TEST(PwlTest, TestArithmetic) {
    const auto S = PiecewiseLinearFunction::createLinear(1.0, 0.0);  // f(x) = x
    const auto K = PiecewiseLinearFunction::createConstant(50.0);

    EXPECT_TRUE(near((S + K)(30.0), 80.0)) << "add";
    EXPECT_TRUE(near((S - K)(80.0), 30.0)) << "subtract";
    EXPECT_TRUE(near((-S)(5.0), -5.0)) << "negate";
    EXPECT_TRUE(near((S * PiecewiseLinearFunction::createConstant(3.0))(4.0), 12.0))
        << "multiply by constant";
    EXPECT_TRUE(near((S / PiecewiseLinearFunction::createConstant(2.0))(6.0), 3.0))
        << "divide by constant";

    // merge: sum of two constants collapses to one segment
    const auto sum =
        PiecewiseLinearFunction::createConstant(2.0) + PiecewiseLinearFunction::createConstant(3.0);
    EXPECT_EQ(sum.getSegments().size(), 1) << "merged: sum of constants is one segment";

    // errors
    bool threw = false;
    try {
        auto r = S * S;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    EXPECT_TRUE(threw) << "S * S throws";

    threw = false;
    try {
        auto r = S / PiecewiseLinearFunction::createConstant(0.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    EXPECT_TRUE(threw) << "divide by zero throws";
}

TEST(PwlTest, TestMaxMin) {
    const auto S = PiecewiseLinearFunction::createLinear(1.0, 0.0);
    const auto zero = PiecewiseLinearFunction::createConstant(0.0);

    // call payoff: max(S - 100, 0)
    const auto call =
        PiecewiseLinearFunction::max(S - PiecewiseLinearFunction::createConstant(100.0), zero);
    EXPECT_TRUE(near(call(50.0), 0.0)) << "call: below strike";
    EXPECT_TRUE(near(call(150.0), 50.0)) << "call: above strike";
    EXPECT_EQ(call.getBreakPoints().size(), 1) << "call: one breakpoint";

    // min(200, max(S - 100, 10)) — floor=10, cap at S=200, linear in between
    const auto inner =
        PiecewiseLinearFunction::max(S - PiecewiseLinearFunction::createConstant(100.0),
                                     PiecewiseLinearFunction::createConstant(10.0));
    const auto capped =
        PiecewiseLinearFunction::min(PiecewiseLinearFunction::createConstant(200.0), inner);

    EXPECT_TRUE(near(capped(50.0), 10.0)) << "floored+capped: floor region";
    EXPECT_TRUE(near(capped(150.0), 50.0)) << "floored+capped: linear region";
    EXPECT_TRUE(near(capped(400.0), 200.0)) << "floored+capped: cap region";
    EXPECT_EQ(capped.getBreakPoints().size(), 2) << "floored+capped: two breakpoints";
}