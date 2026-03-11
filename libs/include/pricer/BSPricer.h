#pragma once

#include "PayoffPricer.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace pricer {

/**
 * Price piecewise linear payoff by applying BlackScholes formula to each segment.
 */
class BSPricer final : public PayoffPricer, public payoff::PayoffVisitor<double> {
   public:
    explicit BSPricer(const market::Market& market) : _market(market) {}
    double price(const payoff::PayoffNodePtr& _payoff) override { return evaluate(_payoff); }

   private:
    const market::Market& _market;
    double visit(const payoff::CashPayment& node) override;
    double visit(const payoff::CombinedPayment& node) override {
        return evaluate(node.getLeftPtr()) + evaluate(node.getRightPtr());
    }
    double visit(const payoff::MultiplyPayment& node) override {
        return node.multiplier() * evaluate(node.getPaymentPtr());
    }
    static double priceSegment(const numerics::linear::Segment& segment, double dF,
                               const market::BSVolSlice& bsVolSlice);
};

inline double bsPricer(const payoff::PayoffNodePtr& payment, const market::Market& market) {
    return BSPricer(market).price(payment);
}
}  // namespace pricer