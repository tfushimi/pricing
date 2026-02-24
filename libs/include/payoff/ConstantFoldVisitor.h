#pragma once

#include "PayoffNode.h"

namespace payoff {

class ConstantFoldVisitor final : public PayoffVisitor<PayoffNodePtr> {

public:
    ConstantFoldVisitor() = default;
    ~ConstantFoldVisitor() override = default;

    PayoffNodePtr visit(const Fixing& node) override {

        return std::make_shared<Fixing>(node.getSymbol(), node.getDate());
    }

    PayoffNodePtr visit(const Constant& node) override {

        return std::make_shared<Constant>(node.getValue());
    }

    PayoffNodePtr visit(const Sum& node) override {
        auto left  = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());
        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return x + y; });
        }
        return std::make_shared<Sum>(left, right);
    }

    PayoffNodePtr visit(const Multiply& node) override;

    PayoffNodePtr visit(const Divide& node) override;

    PayoffNodePtr visit(const Max& node) override;

    PayoffNodePtr visit(const Min& node) override;

    PayoffNodePtr visit(const GreaterThan& node) override;

    PayoffNodePtr visit(const GreaterThanOrEqual& node) override;

    PayoffNodePtr visit(const IfThenElse& node) override;

private:
    static bool isConstant(const PayoffNodePtr& node) {
        return dynamic_cast<const Constant*>(node.get()) != nullptr;
    }

    static bool isConstantValue(const PayoffNodePtr& node, const double value) {
        auto* c = dynamic_cast<const Constant*>(node.get());
        return c != nullptr && c->getValue() == value;
    }

    static double getValue(const PayoffNodePtr& node) {
        return dynamic_cast<const Constant*>(node.get())->getValue();
    }

    template<typename F>
    static PayoffNodePtr fold(const PayoffNodePtr& left, const PayoffNodePtr& right,  F f) {
        return std::make_shared<Constant>(f(getValue(left), getValue(right)));
    }
};
}