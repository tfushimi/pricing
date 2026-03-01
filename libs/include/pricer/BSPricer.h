#pragma once

#include "PayoffPricer.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/PayoffNode.h"

namespace pricer {
class BSPricer final : public PayoffPricer {
   public:
    double price(const payoff::PayoffNodePtr& payoff, const market::Market& market) override;

   private:
    static double priceSegment(const numerics::linear::Segment& segment, double dF,
                               const market::BSVolSlice& bsVolSlice);
    static double safeEndPoint(double slope, double intercept, double endpoint);
};

inline double bsPrice(const payoff::PayoffNodePtr& payoff, const market::Market& market) {
    return BSPricer().price(payoff, market);
}
}  // namespace pricer