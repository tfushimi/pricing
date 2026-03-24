#include <set>

#include "payoff/Observable.h"
#include "payoff/Payoff.h"
#include "payoff/Transforms.h"

namespace payoff {
namespace {

class FixingCollector final : public ObservableVisitor<void>, public PayoffVisitor<void> {
   public:
    // disambiguate evaluate
    using ObservableVisitor::evaluate;
    using PayoffVisitor::evaluate;

    FixingCollector() = default;
    ~FixingCollector() override = default;

    std::set<Fixing> getFixings() const { return _fixings; }

   protected:
    void visit(const Fixing& node) override { _fixings.insert(node); }

    void visit(const Constant&) override {
        // no-op
    }

    void visit(const Add& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Sum& node) override {
        for (const auto& element : node) {
            evaluate(element);
        }
    }

    void visit(const Multiply& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Divide& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Max& node) override {
        for (const auto& element : node) {
            evaluate(element);
        }
    }

    void visit(const Min& node) override {
        for (const auto& element : node) {
            evaluate(element);
        }
    }

    void visit(const GreaterThan& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const GreaterThanOrEqual& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const IfThenElse& node) override {
        evaluate(node.getCond());
        evaluate(node.getThen());
        evaluate(node.getElse());
    }

    void visit(const CashPayment& node) override { evaluate(node.getAmount()); }

    void visit(const CombinedPayment& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const BranchPayment& node) override {
        evaluate(node.getCondition());
        evaluate(node.getThenPayoff());
        evaluate(node.getElsePayoff());
    }

   private:
    std::set<Fixing> _fixings{};
};
}  // namespace

std::set<Fixing> getFixings(const ObservableNodePtr& observable) {
    FixingCollector collector;
    collector.evaluate(observable);
    return collector.getFixings();
}

std::set<Fixing> getFixings(const PayoffNodePtr& payoff) {
    FixingCollector collector;
    collector.evaluate(payoff);
    return collector.getFixings();
}

std::set<Fixing> getFixings(const PayoffNode& payoff) {
    FixingCollector collector;
    collector.evaluate(payoff);
    return collector.getFixings();
}
};  // namespace payoff