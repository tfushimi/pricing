#pragma once

#include "PayoffNode.h"

namespace payoff {

/**
 * Recursively fold Constant nodes to simply PayoffNode tree
 */
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
        auto left = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return x + y; });
        }

        return std::make_shared<Sum>(left, right);
    }

    PayoffNodePtr visit(const Multiply& node) override {
        auto left = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return x * y; });
        }

        // special case: multiply by 0 or 1
        if (isConstantValue(left, 0.0) || isConstantValue(right, 0.0)) {
            return std::make_shared<Constant>(0.0);
        }

        if (isConstantValue(left, 1.0)) {
            return right;
        }

        if (isConstantValue(right, 1.0)) {
            return left;
        }

        return std::make_shared<Multiply>(left, right);
    }

    PayoffNodePtr visit(const Divide& node) override {
        auto left = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            if (isConstantValue(right, 0.0)) {
                throw std::invalid_argument("ConstantFoldVisitor: division by zero");
            }

            return fold(left, right, [](const double x, const double y) { return x / y; });
        }

        if (isConstantValue(right, 1.0)) {
            return left;
        }

        return std::make_shared<Divide>(left, right);
    }

    PayoffNodePtr visit(const Max& node) override {
        auto left = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return std::max(x, y); });
        }

        return std::make_shared<Max>(left, right);
    }

    PayoffNodePtr visit(const Min& node) override {
        auto left = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return std::min(x, y); });
        }

        return std::make_shared<Min>(left, right);
    }

    PayoffNodePtr visit(const GreaterThan& node) override {
        auto left = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right,
                        [](const double x, const double y) { return x > y ? 1.0 : 0.0; });
        }

        return std::make_shared<GreaterThan>(left, right);
    }

    PayoffNodePtr visit(const GreaterThanOrEqual& node) override {
        auto left = evaluate(node.getLeft());
        auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right,
                        [](const double x, const double y) { return x >= y ? 1.0 : 0.0; });
        }

        return std::make_shared<GreaterThanOrEqual>(left, right);
    }

    PayoffNodePtr visit(const IfThenElse& node) override {
        auto cond = evaluate(node.getCond());
        auto then_ = evaluate(node.getThen());
        auto else_ = evaluate(node.getElse());

        // If condition is known at fold time, pick the branch
        if (isConstant(cond)) {
            return getValue(cond) > 0.0 ? then_ : else_;
        }

        return std::make_shared<IfThenElse>(cond, then_, else_);
    }

   private:
    static bool isConstant(const PayoffNodePtr& node) {
        return dynamic_cast<const Constant*>(node.get()) != nullptr;
    }

    static bool isConstantValue(const PayoffNodePtr& node, const double value) {
        auto* c = dynamic_cast<const Constant*>(node.get());
        return c != nullptr && std::abs(c->getValue() - value) < 1e-10;
    }

    static double getValue(const PayoffNodePtr& node) {
        return dynamic_cast<const Constant*>(node.get())->getValue();
    }

    template <typename F>
    static PayoffNodePtr fold(const PayoffNodePtr& left, const PayoffNodePtr& right, F&& f) {
        return std::make_shared<Constant>(std::forward<F>(f)(getValue(left), getValue(right)));
    }
};
}  // namespace payoff