#pragma once

#include "market/Market.h"
#include "payoff/Payoff.h"

namespace pricer {
class PayoffPricer {
   public:
    PayoffPricer() = default;
    virtual ~PayoffPricer() = default;

    // TODO maybe no need to pass market here
    virtual double price(const payoff::PayoffNodePtr& payment, const market::Market& market) = 0;
};

}  // namespace pricer