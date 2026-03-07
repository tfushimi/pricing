#pragma once
#include <set>

#include "common/types.h"
#include "market/Market.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "payoff/PayoffNode.h"
#include "payoff/Payment.h"

namespace payoff {

// Convert payoff to piecewise linear function
numerics::linear::PiecewiseLinearFunction toPiecewiseLinearFunction(const PayoffNodePtr& payoff);

// Substitute observed fixings with constants and simplify constant expressions
PayoffNodePtr applyMarket(const PayoffNodePtr& payoff, const market::Market& market);

// Simplify constant subexpressions
PayoffNodePtr foldConstants(const PayoffNodePtr& payoff);

// Find all fixings in the payoff
std::set<Fixing> getFixings(const PayoffNodePtr& payoff);
std::set<Fixing> getFixings(const PaymentNodePtr& payment);

// Find all symbols and fixingDates in the payoff
inline std::pair<std::set<std::string>, std::vector<Date>> getSymbolsAndFixingDatesInner(
    const std::set<Fixing>& fixings) {
    std::set<std::string> symbols;
    std::vector<Date> fixingDates;
    fixingDates.reserve(fixingDates.size());
    for (const auto& fixing : fixings) {
        symbols.emplace(fixing.getSymbol());
        fixingDates.push_back(fixing.getDate());
    }

    return {symbols, fixingDates};
}

inline std::pair<std::set<std::string>, std::vector<Date>> getSymbolsAndFixingDates(
    const PayoffNodePtr& payoff) {
    const auto fixings = getFixings(payoff);
    return getSymbolsAndFixingDatesInner(fixings);
}

inline std::pair<std::set<std::string>, std::vector<Date>> getSymbolsAndFixingDates(
    const PaymentNodePtr& payment) {
    const auto fixings = getFixings(payment);
    return getSymbolsAndFixingDatesInner(fixings);
}

// Substitute fixings with MC sample and simplify constant expressions
Sample applyFixings(const PayoffNodePtr& payoff, const Scenario& scenario);
}  // namespace payoff