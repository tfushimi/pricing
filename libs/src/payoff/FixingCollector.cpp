#include <set>

#include "payoff/PayoffNode.h"
#include "payoff/Transforms.h"

namespace payoff {
namespace {

class FixingCollector final : public PayoffVisitor<void> {
   public:
    FixingCollector() = default;
    ~FixingCollector() override = default;

    std::set<Fixing> getFixings() const { return _fixings; }

   protected:
    void visit(const Fixing& node) override { _fixings.insert(node); }

    void visit(const Constant&) override {
        // no-op
    }

    void visit(const Sum& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
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
        evaluate(node.getLeft());
        evaluate(node.getRight());
    }

    void visit(const Min& node) override {
        evaluate(node.getLeft());
        evaluate(node.getRight());
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

   private:
    std::set<Fixing> _fixings;
};
}  // namespace

std::set<Fixing> getFixings(const PayoffNodePtr& payoff) {
    FixingCollector collector;
    collector.evaluate(payoff);
    return collector.getFixings();
}
};  // namespace payoff