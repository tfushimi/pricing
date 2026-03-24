#include "payoff/Observable.h"
#include "payoff/Transforms.h"

namespace payoff {
namespace {

/**
 * Recursively fold Constant nodes to simply PayoffNode tree
 */
class ConstantFold final : public ObservableVisitor<ObservableNodePtr> {
   public:
    ConstantFold() = default;
    ~ConstantFold() override = default;

   protected:
    ObservableNodePtr visit(const Fixing& node) override {
        return std::make_shared<Fixing>(node.getSymbol(), node.getDate());
    }

    ObservableNodePtr visit(const Constant& node) override {
        return std::make_shared<Constant>(node.getValue());
    }

    ObservableNodePtr visit(const Add& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return x + y; });
        }

        return std::make_shared<Add>(left, right);
    }

    ObservableNodePtr visit(const Sum& node) override {
        std::vector<ObservableNodePtr> result;
        double sum = 0;
        for (const auto& element : node) {
            const auto newElement = evaluate(element);
            if (isConstant(newElement)) {
                sum += getValue(newElement);
            } else {
                result.push_back(newElement);
            }
        }

        // entire Sum is reduced to a constant
        if (result.empty()) {
            return std::make_shared<Constant>(sum);
        }

        if (sum != 0.0) {
            result.push_back(std::make_shared<Constant>(sum));
        }

        if (result.size() == 1) {
            return result[0];
        }

        return std::make_shared<Sum>(std::move(result));
    }

    ObservableNodePtr visit(const Multiply& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

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

    ObservableNodePtr visit(const Divide& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

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

    ObservableNodePtr visit(const Max& node) override {
        std::vector<ObservableNodePtr> result;

        double maxValue = -std::numeric_limits<double>::infinity();
        for (const auto& element : node) {
            const auto newElement = evaluate(element);
            if (isConstant(newElement)) {
                maxValue = std::max(maxValue, getValue(newElement));
            } else {
                result.push_back(element);
            }
        }

        if (result.empty()) {
            return std::make_shared<Constant>(maxValue);
        }

        if (maxValue != -std::numeric_limits<double>::infinity()) {
            result.push_back(std::make_shared<Constant>(maxValue));
        }

        if (result.size() == 1) {
            return result[0];
        }

        return std::make_shared<Max>(std::move(result));
    }

    ObservableNodePtr visit(const Min& node) override {
        std::vector<ObservableNodePtr> result;

        double minValue = std::numeric_limits<double>::infinity();
        for (const auto& element : node) {
            const auto newElement = evaluate(element);
            if (isConstant(newElement)) {
                minValue = std::min(minValue, getValue(newElement));
            } else {
                result.push_back(element);
            }
        }

        if (result.empty()) {
            return std::make_shared<Constant>(minValue);
        }

        if (minValue != std::numeric_limits<double>::infinity()) {
            result.push_back(std::make_shared<Constant>(minValue));
        }

        if (result.size() == 1) {
            return result[0];
        }

        return std::make_shared<Min>(std::move(result));
    }

    ObservableNodePtr visit(const GreaterThan& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right,
                        [](const double x, const double y) { return x > y ? 1.0 : 0.0; });
        }

        return std::make_shared<GreaterThan>(left, right);
    }

    ObservableNodePtr visit(const GreaterThanOrEqual& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right,
                        [](const double x, const double y) { return x >= y ? 1.0 : 0.0; });
        }

        return std::make_shared<GreaterThanOrEqual>(left, right);
    }

    ObservableNodePtr visit(const IfThenElse& node) override {
        const auto cond = evaluate(node.getCond());
        const auto then_ = evaluate(node.getThen());
        const auto else_ = evaluate(node.getElse());

        // If condition is known at fold time, pick the branch
        if (isConstant(cond)) {
            return getValue(cond) > 0.0 ? then_ : else_;
        }

        return std::make_shared<IfThenElse>(cond, then_, else_);
    }

   private:
    static bool isConstant(const ObservableNodePtr& node) {
        return dynamic_cast<const Constant*>(node.get()) != nullptr;
    }

    static bool isConstantValue(const ObservableNodePtr& node, const double value) {
        auto* c = dynamic_cast<const Constant*>(node.get());
        return c != nullptr && std::abs(c->getValue() - value) < 1e-10;
    }

    static double getValue(const ObservableNodePtr& node) {
        return dynamic_cast<const Constant*>(node.get())->getValue();
    }

    template <typename F>
    static ObservableNodePtr fold(const ObservableNodePtr& left, const ObservableNodePtr& right,
                                  F&& f) {
        return std::make_shared<Constant>(std::forward<F>(f)(getValue(left), getValue(right)));
    }
};
}  // anonymous namespace

ObservableNodePtr foldConstants(const ObservableNodePtr& observable) {
    return ConstantFold().evaluate(observable);
}
}  // namespace payoff