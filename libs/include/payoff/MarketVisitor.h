#pragma once

#include "PayoffNode.h"
#include "market/Market.h"

namespace payoff {

/**
 * Replace Fixing nodes with Constant if observed in Market
 */
class MarketVisitor final : public PayoffVisitor<PayoffNodePtr> {
   public:
    explicit MarketVisitor(const market::Market& market) : _market(market) {}
    ~MarketVisitor() override = default;

   protected:
    // Fixing: replace with Constant if observed, otherwise keep as Fixing
    PayoffNodePtr visit(const Fixing& node) override {
        const auto spot = _market.getPrice(node.getSymbol(), node.getDate());

        if (spot.has_value()) {
            return std::make_shared<Constant>(spot.value());
        }

        return std::make_shared<Fixing>(node.getSymbol(), node.getDate());
    }

    PayoffNodePtr visit(const Constant& node) override {
        return std::make_shared<Constant>(node.getValue());
    }

    PayoffNodePtr visit(const Sum& node) override {
        return std::make_shared<Sum>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PayoffNodePtr visit(const Multiply& node) override {
        return std::make_shared<Multiply>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PayoffNodePtr visit(const Divide& node) override {
        return std::make_shared<Divide>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PayoffNodePtr visit(const Max& node) override {
        return std::make_shared<Max>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PayoffNodePtr visit(const Min& node) override {
        return std::make_shared<Min>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PayoffNodePtr visit(const GreaterThan& node) override {
        return std::make_shared<GreaterThan>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PayoffNodePtr visit(const GreaterThanOrEqual& node) override {
        return std::make_shared<GreaterThanOrEqual>(evaluate(node.getLeft()),
                                                    evaluate(node.getRight()));
    }

    PayoffNodePtr visit(const IfThenElse& node) override {
        return std::make_shared<IfThenElse>(evaluate(node.getCond()), evaluate(node.getThen()),
                                            evaluate(node.getElse()));
    }

   private:
    const market::Market& _market;
};
}  // namespace payoff