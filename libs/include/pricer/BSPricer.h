#pragma once

#include "PayoffPricer.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/Payment.h"
#include "payoff/PayoffNode.h"

namespace pricer {
class BSPricer final : public PayoffPricer {
   public:
    double price(const payoff::PaymentNodePtr& payment, const market::Market& market) override;
    static double price(const payoff::PayoffNodePtr& payoff, const market::Market& market,
                        Date settlementDate);

   private:
    static double priceSegment(const numerics::linear::Segment& segment, double dF,
                               const market::BSVolSlice& bsVolSlice);
};

inline double bsPrice(const payoff::PaymentNodePtr& payment, const market::Market& market) {
    return BSPricer().price(payment, market);
}
}  // namespace pricer