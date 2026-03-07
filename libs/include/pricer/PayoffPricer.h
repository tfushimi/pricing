#pragma once

#include "market/Market.h"
#include "payoff/Payment.h"

namespace pricer {
class PayoffPricer {
   public:
    PayoffPricer() = default;
    virtual ~PayoffPricer() = default;

    virtual double price(const payoff::PaymentNodePtr& payment, const market::Market& market) = 0;
};

}  // namespace pricer