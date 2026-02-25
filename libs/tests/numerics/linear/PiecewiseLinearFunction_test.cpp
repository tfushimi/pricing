#include "numerics/linear/PiecewiseLinearFunction.h"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

#include "numerics/linear/Segment.h"

using namespace numerics::linear;

TEST(PLFTest, TestSegment) {
    // f(x) = 2x + 3
    const Segment s(2.0, 3.0, 0.0, 10.0);
    EXPECT_NEAR(s(5.0), 13.0, 1e-10) << "segment eval";
    EXPECT_TRUE(s.contains(0.0)) << "contains lo (inclusive)";
    EXPECT_TRUE(!s.contains(10.0)) << "excludes hi (open)";
    EXPECT_TRUE(!s.containsInterior(0.0)) << "interior excludes lo";
    EXPECT_TRUE(s.containsInterior(5.0)) << "interior contains midpoint";

    // crossing at x=5
    const Segment f(1.0, 0.0, 0.0, 10.0);
    const Segment g(-1.0, 10.0, 0.0, 10.0);  // cross at x=5
    EXPECT_NEAR(*f.crossing(g), 5.0, 1e-10) << "crossing at x=5";

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

TEST(PLFTest, TestConstruction) {
    // f(x) = 1
    const auto constant = PLF::constant(1.0);
    EXPECT_NEAR(constant(1e10), 1.0, 1e-10) << "constant PLF";
    EXPECT_EQ(constant.getBreakPoints().size(), 0) << "constant has no breakpoints";

    // f(x) = 2x + 1
    const auto linear = PLF::linear(2.0, 1.0);
    EXPECT_NEAR(linear(5.0), 11.0, 1e-10) << "unbounded linear PLF";

    // bounded: f(x) = x on [10, 20), zero outside
    const auto b = PLF::linear(1.0, 0.0, 10.0, 20.0);
    EXPECT_NEAR(b(5.0), 0.0, 1e-10) << "bounded linear: zero left of lo";
    EXPECT_NEAR(b(15.0), 15.0, 1e-10) << "bounded linear: interior";
    EXPECT_NEAR(b(20.0), 0.0, 1e-10) << "bounded linear: zero at hi (open)";
}

TEST(PLFTest, TestArithmetic) {
    const auto S = PLF::linear(1.0, 0.0);  // f(x) = x
    const auto K = PLF::constant(50.0);

    EXPECT_NEAR((S + K)(30.0), 80.0, 1e-10) << "add";
    EXPECT_NEAR((S - K)(80.0), 30.0, 1e-10) << "subtract";
    EXPECT_NEAR((-S)(5.0), -5.0, 1e-10) << "negate";
    EXPECT_NEAR((S * PLF::constant(3.0))(4.0), 12.0, 1e-10) << "multiply by constant";
    EXPECT_NEAR((S / PLF::constant(2.0))(6.0), 3.0, 1e-10) << "divide by constant";

    // merge: sum of two constants collapses to one segment
    const auto sum = PLF::constant(2.0) + PLF::constant(3.0);
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
        auto r = S / PLF::constant(0.0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    EXPECT_TRUE(threw) << "divide by zero throws";
}

TEST(PLFTest, TestGraterThan) {
    const auto S = PLF::linear(1.0, 0.0);  // f(x) = x
    const auto K = PLF::constant(50.0);

    EXPECT_NEAR((S > K)(40), 0.0, 1e-10) << "less than K";
    EXPECT_NEAR((S > K)(50), 0.0, 1e-10) << "at K";
    EXPECT_NEAR((S > K)(60), 1.0, 1e-10) << "greater than K";

    EXPECT_NEAR((S >= K)(40), 0.0, 1e-10) << "less than K";
    EXPECT_NEAR((S >= K)(50), 1.0, 1e-10) << "at K";
    EXPECT_NEAR((S >= K)(60), 1.0, 1e-10) << "greater than K";

    EXPECT_NEAR((PLF::constant(3.0) > PLF::constant(2.0))(-1e10), 1.0, 1e-10);
    EXPECT_NEAR((PLF::constant(3.0) > PLF::constant(2.0))(0.0), 1.0, 1e-10);
    EXPECT_NEAR((PLF::constant(3.0) > PLF::constant(2.0))(1e10), 1.0, 1e-10);
}

TEST(PLFTest, TestIfThenElse) {
    const auto S = PLF::linear(1.0, 0.0);  // f(x) = x
    const auto K = PLF::constant(100.0);

    const auto call = PLF::ite(S > K, S - K, PLF::constant(0.0));

    EXPECT_NEAR(call(50.0), 0.0, 1e-10) << "call: below strike";
    EXPECT_NEAR(call(150.0), 50.0, 1e-10) << "call: above strike";
    EXPECT_EQ(call.getBreakPoints().size(), 1) << "call: one breakpoint";
}

TEST(PLFTest, TestMaxMin) {
    const auto S = PLF::linear(1.0, 0.0);
    const auto zero = PLF::constant(0.0);

    // call payoff: max(S - 100, 0)
    const auto call = PLF::max(S - PLF::constant(100.0), zero);
    EXPECT_NEAR(call(50.0), 0.0, 1e-10) << "call: below strike";
    EXPECT_NEAR(call(150.0), 50.0, 1e-10) << "call: above strike";
    EXPECT_EQ(call.getBreakPoints().size(), 1) << "call: one breakpoint";

    // min(200, max(S - 100, 10)) — floor=10, cap at S=200, linear in between
    const auto inner = PLF::max(S - PLF::constant(100.0), PLF::constant(10.0));
    const auto capped = PLF::min(PLF::constant(200.0), inner);

    EXPECT_NEAR(capped(50.0), 10.0, 1e-10) << "floored+capped: floor region";
    EXPECT_NEAR(capped(150.0), 50.0, 1e-10) << "floored+capped: linear region";
    EXPECT_NEAR(capped(400.0), 200.0, 1e-10) << "floored+capped: cap region";
    EXPECT_EQ(capped.getBreakPoints().size(), 2) << "floored+capped: two breakpoints";
}