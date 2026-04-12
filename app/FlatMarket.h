#pragma once

#include "common/Date.h"
#include "market/SimpleMarket.h"

// Creates a zero-rate, zero-dividend market with flat (constant) volatility.
// Used for book examples that isolate model differences from market data assumptions.
inline market::SimpleMarket makeFlatMarket(const calendar::Date pricingDate,
                                           const std::string& symbol, const double spot,
                                           const double vol = 0.2) {
    return market::SimpleMarket{pricingDate, symbol, spot, 0.0, 0.0, vol};
}
