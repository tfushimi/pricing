#pragma once

#include "PayoffNode.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/types.h"

namespace payoff {

class PLVisitor final : public PayoffVisitor<PL> {
   public:
    PL visit(const Fixing& node) override {
        // TODO store fixing date for validation
        return PL::linear(1.0, 0.0);
    }

    PL visit(const Constant& node) override { return PL::constant(node.getValue()); }

    PL visit(const Sum& node) override {
        return evaluate(node.getLeft()) + evaluate(node.getRight());
    }

    PL visit(const Multiply& node) override {
        return evaluate(node.getLeft()) * evaluate(node.getRight());
    }

    PL visit(const Divide& node) override {
        return evaluate(node.getLeft()) / evaluate(node.getRight());
    }

    PL visit(const Max& node) override {
        return PL::max(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PL visit(const Min& node) override {
        return PL::min(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    PL visit(const GreaterThan& node) override {
        return evaluate(node.getLeft()) > evaluate(node.getRight());
    }

    PL visit(const GreaterThanOrEqual& node) override {
        return evaluate(node.getLeft()) >= evaluate(node.getRight());
    }

    PL visit(const IfThenElse& node) override {
        return PL::ite(evaluate(node.getCond()), evaluate(node.getThenPtr()),
                       evaluate(node.getElse()));
    }
};
}  // namespace payoff