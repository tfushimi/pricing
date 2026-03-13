#pragma once

#include "PayoffPricer.h"
#include "common/types.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace pricer {

/**
 * Price piecewise linear payoff by applying Heston formula to each segment.
 */
class HestonPricer final : public PayoffPricer, public payoff::PayoffVisitor<double> {
   public:
    explicit HestonPricer(const market::Market& market, const HestonParams& params)
        : _market(market), _params(params) {}
    double price(const payoff::PayoffNodePtr& _payoff) override { return evaluate(_payoff); }

   private:
    const market::Market& _market;
    const HestonParams& _params;
    double visit(const payoff::CashPayment& node) override;
    double visit(const payoff::CombinedPayment& node) override {
        return evaluate(node.getLeftPtr()) + evaluate(node.getRightPtr());
    }
    double visit(const payoff::MultiplyPayment& node) override {
        return node.multiplier() * evaluate(node.getPaymentPtr());
    }
    double priceSegment(const numerics::linear::Segment& segment, double dF,
                        const market::BSVolSlice& bsVolSlice) const;
};

inline double hestonPricer(const payoff::PayoffNodePtr& payment, const market::Market& market,
                           const HestonParams& params) {
    return HestonPricer(market, params).price(payment);
}
}  // namespace pricer