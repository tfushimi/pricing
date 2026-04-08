#include <ranges>

#include "market/Market.h"
#include "mc/ProcessStateStepper.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace market;
using namespace mc;
using namespace calendar;

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

        // TODO verify symbol matches _scenario
        return _scenario.at(fixingDate);
    }

    Sample visit(const Constant& node) override { return Sample(node.getValue(), _dim); }

    Sample visit(const Add& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return left + right;
    }

    Sample visit(const Sum& node) override {
        Sample result = evaluate(*node.begin());
        for (auto it = std::next(node.begin()); it != node.end(); ++it) {
            result += evaluate(*it);
        }
        return result;
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
        Sample result = evaluate(*node.begin());
        for (auto it = std::next(node.begin()); it != node.end(); ++it) {
            result = elementwiseMax(result, evaluate(*it));
        }
        return result;
    }

    Sample visit(const Min& node) override {
        Sample result = evaluate(*node.begin());
        for (auto it = std::next(node.begin()); it != node.end(); ++it) {
            result = elementwiseMin(result, evaluate(*it));
        }
        return result;
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
    static Sample elementwiseMin(const Sample& left, const Sample& right) {
        return (left + right - abs(left - right)) * 0.5;
    }

    static Sample elementwiseMax(const Sample& left, const Sample& right) {
        return (left + right + abs(left - right)) * 0.5;
    }
};

class ApplyPayoffFixings final : public PayoffVisitor<Sample> {
   public:
    explicit ApplyPayoffFixings(const Market& market, const Scenario& scenario)
        : _market(market), _scenario(scenario) {}
    ~ApplyPayoffFixings() override = default;

   protected:
    Sample visit(const CashPayment& node) override {
        const auto sample = applyFixings(node.getAmount(), _scenario);
        return sample * _market.getDiscountFactor(node.getSettlementDate());
    }

    Sample visit(const CombinedPayment& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return left + right;
    }

    Sample visit(const BranchPayment& node) override {
        const auto cond = applyFixings(node.getCondition(), _scenario);
        const auto then_ = evaluate(node.getThenPayoff());
        const auto else_ = evaluate(node.getElsePayoff());

        Sample result(cond.size());
        for (size_t i = 0; i < cond.size(); ++i) {
            result[i] = cond[i] > 0.0 ? then_[i] : else_[i];
        }

        return result;
    }

   private:
    const Market& _market;
    const Scenario& _scenario;
};
}  // namespace

Sample applyFixings(const ObservableNode& observable, const Scenario& scenario) {
    return ApplyFixings(scenario).evaluate(observable);
}

Sample applyFixings(const ObservableNodePtr& observable, const Scenario& scenario) {
    return ApplyFixings(scenario).evaluate(observable);
}

Sample applyFixings(const PayoffNodePtr& payoff, const Market& market, const Scenario& scenario) {
    return ApplyPayoffFixings(market, scenario).evaluate(payoff);
}
}  // namespace payoff