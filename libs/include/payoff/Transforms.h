#pragma once
#include <set>

#include "common/types.h"
#include "market/Market.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "payoff/PayoffNode.h"

namespace payoff {

// Convert payoff to piecewise linear function
numerics::linear::PiecewiseLinearFunction toPiecewiseLinearFunction(const PayoffNodePtr& payoff);

// Substitute observed fixings with constants and simplify constant expressions
PayoffNodePtr applyMarket(const PayoffNodePtr& payoff, const market::Market& market);

// Simplify constant subexpressions
PayoffNodePtr foldConstants(const PayoffNodePtr& payoff);

// Find all fixings in the payoff
std::set<Fixing> getFixings(const PayoffNodePtr& payoff);

// Substitute fixings with MC sample and simplify constant expressions
Sample applyFixings(const PayoffNodePtr& payoff, const Scenario& scenario);
}  // namespace payoff