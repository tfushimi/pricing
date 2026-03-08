#pragma once

#include "payoff/Payoff.h"

namespace pricer {
class PayoffPricer {
   public:
    PayoffPricer() = default;
    virtual ~PayoffPricer() = default;

    virtual double price(const payoff::PayoffNodePtr& _payoff) = 0;
};

}  // namespace pricer