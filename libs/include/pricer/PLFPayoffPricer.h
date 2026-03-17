#pragma once

#include "PayoffPricer.h"
#include "market/BSVolSlice.h"
#include "market/Market.h"
#include "numerics/linear/Segment.h"
#include "payoff/Observable.h"
#include "payoff/Payoff.h"

namespace pricer {

/**
 * Price piecewise linear payoff by applying call and digital call formula to each segment.
 */
class PLFPayoffPricer : public PayoffPricer, public payoff::PayoffVisitor<double> {
   public:
    explicit PLFPayoffPricer(const market::Market& market) : _market(market) {}
    double price(const payoff::PayoffNodePtr& _payoff) override { return evaluate(_payoff); }

   private:
    const market::Market& _market;
    double visit(const payoff::CashPayment& node) override;
    double visit(const payoff::CombinedPayment& node) override {
        return evaluate(node.getLeft()) + evaluate(node.getRight());
    }
    double visit(const payoff::BranchPayment&) override {
        throw std::runtime_error("PiecewiseLinearPayoff should not have BranchPayment");
    }
    double priceSegment(const numerics::linear::Segment& segment, double dF,
                        const market::BSVolSlice& bsVolSlice);
    virtual double callFormula(double F, double K, double T, double dF,
                               const market::BSVolSlice& bsVolSlice) const = 0;
    virtual double digitalCallFormula(double F, double K, double T, double dF,
                                      const market::BSVolSlice& bsVolSlice) const = 0;
};
}  // namespace pricer