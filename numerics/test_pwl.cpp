#include <cmath>
#include <iostream>
#include <stdexcept>

#include "Segment.h"
#include "PiecewiseLinearFunction.h"

using namespace numerics::pwl;

static void check(const bool condition, const char* msg) {
    if (!condition) {
        std::cerr << "FAIL: " << msg << "\n";
        std::exit(1);
    }
    std::cout << "PASS: " << msg << "\n";
}

static bool near(const double a, const double b, double tol = 1e-10) {
    return std::abs(a - b) < tol;
}

void test_segment() {
    // f(x) = 2x + 3
    const Segment s(2.0, 3.0, 0.0, 10.0);
    check(near(s(5.0), 13.0), "segment eval");
    check(s.contains(0.0),  "contains lo (inclusive)");
    check(!s.contains(10.0), "excludes hi (open)");
    check(!s.containsInterior(0.0), "interior excludes lo");
    check(s.containsInterior(5.0), "interior contains midpoint");

    // crossing at x=5
    const Segment f(1.0,  0.0,  0.0, 10.0);
    const Segment g(-1.0, 10.0, 0.0, 10.0);  // cross at x=5
    check(near(*f.crossing(g), 5.0),   "crossing at x=5");

    // no crossing
    const Segment h(1.0, 5.0, 0.0, 10.0);   // parallel to f
    check(!f.crossing(h).has_value(),  "parallel: no crossing");

    // last segment [lo, +inf) contains everything >= lo
    const Segment last(0.0, 1.0, 100.0, POS_INF);
    check(last.contains(1e15), "last segment contains large value");

    // invalid construction
    bool threw = false;
    try {
        Segment bad(0.0, 0.0, 10.0, 5.0);
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "lo >= hi throws");
}

void test_construction() {
    // f(x) = 1
    const auto constant = PiecewiseLinearFunction::createConstant(1.0);
    check(near(constant(1e10), 1.0), "constant PLF");
    check(constant.getBreakPoints().size() == 0, "constant has no breakpoints");

    // f(x) = 2x + 1
    const auto linear = PiecewiseLinearFunction::createLinear(2.0, 1.0);
    check(near(linear(5.0), 11.0), "unbounded linear PLF");

    // bounded: f(x) = x on [10, 20), zero outside
    const auto b = PiecewiseLinearFunction::createLinear(1.0, 0.0, 10.0, 20.0);
    check(near(b(5.0),  0.0),  "bounded linear: zero left of lo");
    check(near(b(15.0), 15.0), "bounded linear: interior");
    check(near(b(20.0), 0.0),  "bounded linear: zero at hi (open)");
}

void test_arithmetic() {
    const auto S = PiecewiseLinearFunction::createLinear(1.0, 0.0);  // f(x) = x
    const auto K = PiecewiseLinearFunction::createConstant(50.0);

    check(near((S + K)(30.0),  80.0), "add");
    check(near((S - K)(80.0),  30.0), "subtract");
    check(near((-S)(5.0),      -5.0), "negate");
    check(near((S * PiecewiseLinearFunction::createConstant(3.0))(4.0), 12.0), "multiply by constant");
    check(near((S / PiecewiseLinearFunction::createConstant(2.0))(6.0),  3.0), "divide by constant");

    // merge: sum of two constants collapses to one segment
    const auto sum = PiecewiseLinearFunction::createConstant(2.0)
             + PiecewiseLinearFunction::createConstant(3.0);
    check(sum.getSegments().size() == 1, "merged: sum of constants is one segment");

    // errors
    bool threw = false;
    try {
        auto r = S * S;
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "S * S throws");

    threw = false;
    try {
        auto r = S / PiecewiseLinearFunction::createConstant(0.0);
    }
    catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "divide by zero throws");
}

void test_max_min() {
    const auto S = PiecewiseLinearFunction::createLinear(1.0, 0.0);
    const auto zero = PiecewiseLinearFunction::createConstant(0.0);

    // call payoff: max(S - 100, 0)
    const auto call = PiecewiseLinearFunction::max(
                    S - PiecewiseLinearFunction::createConstant(100.0), zero);
    check(near(call(50.0),   0.0), "call: below strike");
    check(near(call(150.0), 50.0), "call: above strike");
    check(call.getBreakPoints().size() == 1, "call: one breakpoint");

    // min(200, max(S - 100, 10)) — floor=10, cap at S=200, linear in between
    const auto inner = PiecewiseLinearFunction::max(
                     S - PiecewiseLinearFunction::createConstant(100.0),
                     PiecewiseLinearFunction::createConstant(10.0));
    const auto capped = PiecewiseLinearFunction::min(
                 PiecewiseLinearFunction::createConstant(200.0), inner);

    check(near(capped(50.0),   10.0), "floored+capped: floor region");
    check(near(capped(150.0),  50.0), "floored+capped: linear region");
    check(near(capped(400.0), 200.0), "floored+capped: cap region");
    check(capped.getBreakPoints().size() == 2, "floored+capped: two breakpoints");
}

int main() {
    std::cout << "Segment" << std::endl;
    test_segment();

    std::cout << "Construction" << std::endl;
    test_construction();

    std::cout << "Arithmetic" << std::endl;
    test_arithmetic();

    std::cout << "Max/Min" << std::endl;
    test_max_min();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}