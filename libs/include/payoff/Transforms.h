#pragma once
#include "payoff/PayoffNode.h"
#include "market/Market.h"

namespace payoff {
// Substitute observed fixings with constants in market
PayoffNodePtr applyMarket(const PayoffNodePtr& payoff,
                               const market::Market& market);

// Simplify constant subexpressions
PayoffNodePtr foldConstants(const PayoffNodePtr& payoff);

// Common pattern: apply both in sequence
inline PayoffNodePtr simplify(const PayoffNodePtr& payoff,
                              const market::Market& market) {
  return foldConstants(applyMarket(payoff, market));
}
}