#include <optional>

#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/types.h"
#include "payoff/PayoffNode.h"
#include "payoff/Transforms.h"

using namespace numerics::linear;

namespace payoff {
namespace {
/**
 * Convert PayoffNodePtr tree into a piecewise liear function
 */
class PiecewiseLinearFunctionVisitor final : public PayoffVisitor<PLF> {
   protected:
    PLF visit(const Fixing& node) override {
        if (_symbol.has_value() && _symbol != node.getSymbol()) {
            throw std::invalid_argument("PLPayoff cannot have more than one symbol");
        }

        _symbol = node.getSymbol();

        if (_fixingDate.has_value() && _fixingDate != node.getDate()) {
            throw std::invalid_argument("PLPayoff cannot have more than one date");
        }

        _fixingDate = node.getDate();

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

   private:
    std::optional<std::string> _symbol = std::nullopt;
    std::optional<Date> _fixingDate = std::nullopt;
};
}  // namespace

PLF toPiecewiseLinearFunction(const PayoffNodePtr& payoff) {
    return PiecewiseLinearFunctionVisitor().evaluate(payoff);
}
}  // namespace payoff