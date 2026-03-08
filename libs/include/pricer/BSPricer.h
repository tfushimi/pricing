#pragma once

#include "PayoffPricer.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace pricer {
class BSPricer final : public PayoffPricer, public payoff::PayoffVisitor<double> {
   public:
    explicit BSPricer(const market::Market& market) : _market(market) {}
    double price(const payoff::PayoffNodePtr& _payoff, const market::Market&) override {
        return evaluate(_payoff);
    }

   private:
    const market::Market& _market;
    double visit(const payoff::CashPayment& node) override;
    double visit(const payoff::CombinedPayment& node) override;
    double visit(const payoff::MultiplyPayment& node) override;
    static double priceSegment(const numerics::linear::Segment& segment, double dF,
                               const market::BSVolSlice& bsVolSlice);
};

inline double bsPrice(const payoff::PayoffNodePtr& payment, const market::Market& market) {
    return BSPricer(market).price(payment, market);
}
}  // namespace pricer