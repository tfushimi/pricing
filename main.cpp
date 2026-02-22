#include <iostream>

#include "numerics/PiecewiseLinearFunction.h"
#include "numerics/Segment.h"

using namespace numerics::pwl;

int main() {

    // max(x - 100, 0) — call option payoff
    const auto s   = PiecewiseLinearFunction::createLinear(1.0, 0.0);
    const auto k   = PiecewiseLinearFunction::createConstant(100.0);
    const auto payoff   = PiecewiseLinearFunction::max(s - k, PiecewiseLinearFunction::createConstant(0.0));

    std::cout << payoff.toString() << std::endl;

    return 0;
}
