#pragma once

#include "PayoffPricer.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/PayoffNode.h"

namespace pricer {
class BSPricer final : public PayoffPricer {
   public:
    BSPricer() = default;
    BSPricer(const BSPricer&) = delete;
    BSPricer(BSPricer&&) = delete;
    BSPricer& operator=(BSPricer&&) = delete;
    BSPricer& operator=(const BSPricer&) = delete;
    ~BSPricer() override = default;

    double price(const payoff::PayoffNodePtr& payoff, const market::Market& market) override;

   private:
    static double priceSegment(const numerics::linear::Segment& segment, double dF,
                               const market::BSVolSlice& bsVolSlice);
};
}  // namespace pricer