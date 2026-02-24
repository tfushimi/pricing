#pragma once
#include <limits>

namespace numerics::linear {
class PiecewiseLinearFunction;
}
constexpr double NEG_INF = -std::numeric_limits<double>::infinity();
constexpr double POS_INF = std::numeric_limits<double>::infinity();
using PLF = numerics::linear::PiecewiseLinearFunction;
