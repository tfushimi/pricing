#include <ranges>

#include "market/Market.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

namespace payoff {
namespace {

/**
 * Recursively apply Sample to Fixing and fold PayoffNode tree
 */
class ApplyFixings final : public ObservableVisitor<Sample> {
   public:
    explicit ApplyFixings(const Scenario& scenario) : _scenario(scenario) {
        if (scenario.empty()) {
            throw std::invalid_argument("Scenario cannot be empty");
        }

        _dim = _scenario.begin()->second.size();

        if (_dim == 0) {
            throw std::invalid_argument("Sample cannot be empty");
        }

        for (const auto& sample : scenario | std::views::values) {
            if (sample.size() != _dim) {
                throw std::invalid_argument("Sample size doesn't match expected size");
            }
        }
    }

    ~ApplyFixings() override = default;

   protected:
    Sample visit(const Fixing& node) override {
        const Date fixingDate = node.getDate();

        if (!_scenario.contains(fixingDate)) {
            throw std::invalid_argument("Sample not found on " + toString(fixingDate));
        }

        return _scenario.at(fixingDate);
    }

    Sample visit(const Constant& node) override { return Sample(node.getValue(), _dim); }

    Sample visit(const Sum& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return left + right;
    }

    Sample visit(const Multiply& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return left * right;
    }

    Sample visit(const Divide& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return left / right;
    }

    Sample visit(const Max& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return (left + right + abs(left - right)) * 0.5;
    }

    Sample visit(const Min& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return (left + right - abs(left - right)) * 0.5;
    }

    Sample visit(const GreaterThan& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        Sample result(left.size());
        for (size_t i = 0; i < left.size(); ++i) {
            result[i] = left[i] > right[i] ? 1.0 : 0.0;
        }

        return result;
    }

    Sample visit(const GreaterThanOrEqual& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());

        Sample result(left.size());
        for (size_t i = 0; i < left.size(); ++i) {
            result[i] = left[i] >= right[i] ? 1.0 : 0.0;
        }

        return result;
    }

    Sample visit(const IfThenElse& node) override {
        const auto cond = evaluate(node.getCond());
        const auto then_ = evaluate(node.getThen());
        const auto else_ = evaluate(node.getElse());

        Sample result(cond.size());
        for (size_t i = 0; i < cond.size(); ++i) {
            result[i] = cond[i] > 0.0 ? then_[i] : else_[i];
        }

        return result;
    }

   private:
    const Scenario& _scenario;
    std::size_t _dim;
};

class ApplyPayoffFixings final : public PayoffVisitor<Sample> {
   public:
    explicit ApplyPayoffFixings(const market::Market& market, const Scenario& scenario)
        : _discountCurve(market.getDiscountCurve()), _scenario(scenario) {
        if (!_discountCurve) {
            throw std::invalid_argument("DiscountCurve not found");
        }
    }
    ~ApplyPayoffFixings() override = default;

   protected:
    Sample visit(const CashPayment& node) override {
        const auto sample = applyFixings(node.getAmountPtr(), _scenario);
        return sample * _discountCurve->get(node.getSettlementDate());
    }

    Sample visit(const CombinedPayment& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return left + right;
    }

    Sample visit(const MultiplyPayment& node) override {
        const auto sample = evaluate(node.getPayoff());
        return node.multiplier() * sample;
    }

   private:
    std::shared_ptr<const Curve> _discountCurve;
    const Scenario& _scenario;
};
}  // namespace

Sample applyFixings(const ObservableNodePtr& observable, const Scenario& scenario) {
    return ApplyFixings(scenario).evaluate(observable);
}

// TODO add unit tests
Sample applyFixings(const PayoffNodePtr& payoff, const market::Market& market,
                    const Scenario& scenario) {
    return ApplyPayoffFixings(market, scenario).evaluate(payoff);
}
}  // namespace payoff