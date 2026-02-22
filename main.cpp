#include <iostream>

#include "numerics/PiecewiseLinearFunction.h"
#include "numerics/Segment.h"
#include "numerics/types.h"

using namespace numerics::pwl;

bool areAlmostEqual(double a, double b, double epsilon = std::numeric_limits<double>::epsilon()) {
    return std::abs(a - b) <= epsilon * std::max({1.0, std::abs(a), std::abs(b)}); // Relative epsilon method
}

void assertConstant(const PiecewiseLinearFunction& f, const double value) {

    assert(f.getSegments().size() == 1);

    const Segment& segment = f.getSegments()[0];

    assert(segment.getSlope() == 0);
    assert(areAlmostEqual(segment.getIntercept(), value, 1.e-8));
    assert(segment.getLeft() == MIN_VALUE);
    assert(segment.getRight() == MAX_VALUE);
}

void testConstant() {

    const auto constOne = PiecewiseLinearFunction::createConstant(1.0);
    const auto constTwo = PiecewiseLinearFunction::createConstant(2.0);
    const auto constThree = constOne + constTwo;
    const auto constSix = constTwo * constThree;
    const auto constHalf = constOne / constTwo;

    assertConstant(constOne, 1.0);
    assertConstant(constTwo, 2.0);
    assertConstant(constThree, 3.0);
    assertConstant(constSix, 6.0);
    assertConstant(constHalf, 0.5);
}

int main() {

    testConstant();

    return 0;
}
