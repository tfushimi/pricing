#pragma once

#include "market/Market.h"
#include "payoff/PayoffNode.h"

namespace pricer {
class BSPricer {
   public:
    BSPricer() = default;
    BSPricer(const BSPricer&) = delete;
    BSPricer(BSPricer&&) = delete;
    BSPricer& operator=(BSPricer&&) = delete;
    BSPricer& operator=(const BSPricer&) = delete;
    ~BSPricer() = default;

    static double price(const payoff::PayoffNodePtr& payoff, const market::Market& market);

   private:
    static double priceSegment(double slope, double intercept, double lo, double hi, double dF,
                               const market::BSVolSlice& bsVolSlice);
};
}  // namespace pricer