#include <optional>

#include "numerics/linear/PiecewiseLinearFunction.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace numerics::linear;

namespace payoff {
namespace {
/**
 * Convert PayoffNodePtr tree into a piecewise linear function
 */
class ToPiecewiseLinearFunction final : public ObservableVisitor<PiecewiseLinearFunction> {
   protected:
    PiecewiseLinearFunction visit(const Fixing& node) override {
        if (_symbol.has_value() && _symbol != node.getSymbol()) {
            throw std::invalid_argument("PLPayoff cannot have more than one symbol");
        }

        _symbol = node.getSymbol();

        if (_fixingDate.has_value() && _fixingDate != node.getDate()) {
            throw std::invalid_argument("PLPayoff cannot have more than one date");
        }

        _fixingDate = node.getDate();

        return PiecewiseLinearFunction::linear(1.0, 0.0);
    }

    PiecewiseLinearFunction visit(const Constant& node) override {
        return PiecewiseLinearFunction::constant(node.getValue());
    }

    PiecewiseLinearFunction visit(const Add& node) override {
        return evaluate(node.getLeft()) + evaluate(node.getRight());
    }

    PiecewiseLinearFunction visit(const Sum& node) override {
        auto result = PiecewiseLinearFunction::constant(0.0);
        for (const auto& element : node) {
            result += evaluate(element);
        }
        return result;
    }

    PiecewiseLinearFunction visit(const Multiply& node) override {
        return evaluate(node.getLeft()) * evaluate(node.getRight());
    }

    PiecewiseLinearFunction visit(const Divide& node) override {
        return evaluate(node.getLeft()) / evaluate(node.getRight());
    }

    PiecewiseLinearFunction visit(const Max& node) override {
        PiecewiseLinearFunction result = evaluate(*node.begin());
        for (auto it = std::next(node.begin()); it != node.end(); ++it) {
            result = PiecewiseLinearFunction::max(result, evaluate(*it));
        }
        return result;
    }

    PiecewiseLinearFunction visit(const Min& node) override {
        PiecewiseLinearFunction result = evaluate(*node.begin());
        for (auto it = std::next(node.begin()); it != node.end(); ++it) {
            result = PiecewiseLinearFunction::min(result, evaluate(*it));
        }
        return result;
    }

    PiecewiseLinearFunction visit(const GreaterThan& node) override {
        return evaluate(node.getLeft()) > evaluate(node.getRight());
    }

    PiecewiseLinearFunction visit(const GreaterThanOrEqual& node) override {
        return evaluate(node.getLeft()) >= evaluate(node.getRight());
    }

    PiecewiseLinearFunction visit(const IfThenElse& node) override {
        return PiecewiseLinearFunction::ite(evaluate(node.getCondition()), evaluate(node.getThen()),
                                            evaluate(node.getElse()));
    }

   private:
    std::optional<std::string> _symbol = std::nullopt;
    std::optional<Date> _fixingDate = std::nullopt;
};
}  // namespace

PiecewiseLinearFunction toPiecewiseLinearFunction(const ObservableNode& payoff) {
    return ToPiecewiseLinearFunction().evaluate(payoff);
}
PiecewiseLinearFunction toPiecewiseLinearFunction(const ObservableNodePtr& payoff) {
    return ToPiecewiseLinearFunction().evaluate(payoff);
}
}  // namespace payoff