#pragma once

#include <set>

#include "PayoffNode.h"

namespace payoff {

class FixingCollector final : public PayoffVisitor<void> {
   public:
    FixingCollector() = default;
    ~FixingCollector() override = default;

    const std::set<Fixing>& getFixings() const { return _fixings; }

   protected:
    void visit(const Fixing& node) override { _fixings.insert(node); }

    void visit(const Constant& node) override {
        // no-op
    }

    void visit(const Sum& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Multiply& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Divide& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Max& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Min& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const GreaterThan& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const GreaterThanOrEqual& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const IfThenElse& node) override {
        evaluate(node.getCond());
        evaluate(node.getThen());
        evaluate(node.getElse());
    }

   private:
    std::set<Fixing> _fixings;
};

inline std::set<Fixing> collectFixings(const PayoffNodePtr& payoff) {
    FixingCollector collector;
    collector.evaluate(payoff);
    return collector.getFixings();
}
}  // namespace payoff