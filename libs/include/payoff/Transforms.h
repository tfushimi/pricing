#pragma once
#include <set>

#include "market/Market.h"
#include "payoff/PayoffNode.h"

namespace payoff {
// Substitute observed fixings with constants
PayoffNodePtr applyMarket(const PayoffNodePtr& payoff, const market::Market& market);

// Simplify constant subexpressions
PayoffNodePtr foldConstants(const PayoffNodePtr& payoff);

// Common pattern: apply both in sequence
inline PayoffNodePtr simplify(const PayoffNodePtr& payoff, const market::Market& market) {
    return foldConstants(applyMarket(payoff, market));
}

// Find all fixings in the payoff
inline std::set<Fixing> getFixings(const PayoffNodePtr& payoff);
}  // namespace payoff