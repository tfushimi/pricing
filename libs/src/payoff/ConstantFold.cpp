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

    ObservableNodePtr visit(const Sum& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return x + y; });
        }

        return std::make_shared<Sum>(left, right);
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
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return std::max(x, y); });
        }

        return std::make_shared<Max>(left, right);
    }

    ObservableNodePtr visit(const Min& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        if (isConstant(left) && isConstant(right)) {
            return fold(left, right, [](const double x, const double y) { return std::min(x, y); });
        }

        return std::make_shared<Min>(left, right);
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

ObservableNodePtr foldConstants(const ObservableNodePtr& payoff) {
    return ConstantFold().evaluate(payoff);
}
}  // namespace payoff