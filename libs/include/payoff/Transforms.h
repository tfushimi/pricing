#pragma once
#include <set>

#include "common/types.h"
#include "market/Market.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace payoff {

// Convert payoff to piecewise linear function
numerics::linear::PiecewiseLinearFunction toPiecewiseLinearFunction(
    const ObservableNodePtr& payoff);
numerics::linear::PiecewiseLinearFunction toPiecewiseLinearFunction(
    const ObservableNode& payoff);

// Substitute observed fixings with constants and simplify constant expressions
ObservableNodePtr applyMarket(const ObservableNodePtr& observable, const market::Market& market);
ObservableNodePtr applyMarket(const ObservableNode& observable, const market::Market& market);
PayoffNodePtr applyMarket(const PayoffNodePtr& payoff, const market::Market& market);
PayoffNodePtr applyMarket(const PayoffNode& payoff, const market::Market& market);

// specialized method to avoid casting
CashPayment applyMarket(const CashPayment& payoff, const market::Market& market);

// Simplify constant subexpressions
ObservableNodePtr foldConstants(const ObservableNodePtr& observable);

// Find all fixings in the payoff
std::set<Fixing> getFixings(const ObservableNodePtr& observable);
std::set<Fixing> getFixings(const PayoffNodePtr& payoff);
std::set<Fixing> getFixings(const PayoffNode& observable);

// Find all symbols and fixingDates
inline std::pair<std::set<std::string>, std::vector<Date>> getSymbolsAndFixingDatesInner(
    const std::set<Fixing>& fixings) {
    std::set<std::string> symbols;
    std::vector<Date> fixingDates;
    fixingDates.reserve(fixings.size());
    for (const auto& fixing : fixings) {
        symbols.emplace(fixing.getSymbol());
        fixingDates.push_back(fixing.getDate());
    }

    return {symbols, fixingDates};
}

inline std::pair<std::set<std::string>, std::vector<Date>> getSymbolsAndFixingDates(
    const ObservableNodePtr& observable) {
    const auto fixings = getFixings(observable);
    return getSymbolsAndFixingDatesInner(fixings);
}

inline std::pair<std::set<std::string>, std::vector<Date>> getSymbolsAndFixingDates(
    const PayoffNodePtr& payoff) {
    const auto fixings = getFixings(payoff);
    return getSymbolsAndFixingDatesInner(fixings);
}

// Substitute fixings with MC sample and simplify constant expressions
Sample applyFixings(const ObservableNode& observable, const Scenario& scenario);
Sample applyFixings(const ObservableNodePtr& observable, const Scenario& scenario);
Sample applyFixings(const PayoffNodePtr& payoff, const market::Market& market,
                    const Scenario& scenario);
}  // namespace payoff