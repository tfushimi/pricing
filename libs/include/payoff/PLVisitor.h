#pragma once

#include "PayoffNode.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/types.h"

namespace payoff {

class PLVisitor final : public PayoffVisitor<PLF> {
   public:
    PLF visit(const Fixing& node) override {
        // TODO store fixing date for validation
        return PLF::linear(1.0, 0.0);
    }

    PLF visit(const Constant& node) override { return PLF::constant(node.getValue()); }

    PLF visit(const Sum& node) override {
        return evaluate(node.getLeft()) + evaluate(node.getRight());
    }

    PLF visit(const Multiply& node) override {
        return evaluate(node.getLeft()) * evaluate(node.getRight());
    }

    PLF visit(const Divide& node) override {
        return evaluate(node.getLeft()) / evaluate(node.getRight());
    }

    PLF visit(const Max& node) override {
        return PLF::max(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PLF visit(const Min& node) override {
        return PLF::min(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PLF visit(const GreaterThan& node) override {
        return evaluate(node.getLeft()) > evaluate(node.getRight());
    }

    PLF visit(const GreaterThanOrEqual& node) override {
        return evaluate(node.getLeft()) >= evaluate(node.getRight());
    }

    PLF visit(const IfThenElse& node) override {
        return PLF::ite(evaluate(node.getCond()), evaluate(node.getThenPtr()),
                       evaluate(node.getElse()));
    }
};
}  // namespace payoff