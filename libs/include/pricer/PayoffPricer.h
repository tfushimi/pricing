#pragma once

#include "market/Market.h"
#include "payoff/Payoff.h"

namespace pricer {
class PayoffPricer {
   public:
    PayoffPricer() = default;
    virtual ~PayoffPricer() = default;

    virtual double price(const payoff::Payoff& payoff, const market::Market& market) = 0;
};

}  // namespace pricer