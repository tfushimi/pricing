#pragma once

#include "PayoffPricer.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace pricer {
class BSPricer final : public PayoffPricer {
   public:
    double price(const payoff::PayoffNodePtr& payment, const market::Market& market) override;
    static double price(const payoff::ObservableNodePtr& payoff, const market::Market& market,
                        Date settlementDate);

   private:
    static double priceSegment(const numerics::linear::Segment& segment, double dF,
                               const market::BSVolSlice& bsVolSlice);
};

inline double bsPrice(const payoff::PayoffNodePtr& payment, const market::Market& market) {
    return BSPricer().price(payment, market);
}
}  // namespace pricer