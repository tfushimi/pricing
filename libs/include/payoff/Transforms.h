#pragma once
#include <set>

#include "market/Market.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/types.h"
#include "payoff/PayoffNode.h"

namespace payoff {

// Convert payoff to piecewise linear function
numerics::linear::PLF toPiecewiseLinearFunction(const PayoffNodePtr& payoff);

// Substitute observed fixings with constants
PayoffNodePtr applyMarket(const PayoffNodePtr& payoff, const market::Market& market);

// Simplify constant subexpressions
PayoffNodePtr foldConstants(const PayoffNodePtr& payoff);

// Common pattern: apply applyMarket and foldConstants in sequence
inline PayoffNodePtr simplify(const PayoffNodePtr& payoff, const market::Market& market) {
    return foldConstants(applyMarket(payoff, market));
}

// Find all fixings in the payoff
std::set<Fixing> getFixings(const PayoffNodePtr& payoff);
}  // namespace payoff