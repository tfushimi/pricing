#include "market/Market.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

namespace payoff {
namespace {

/**
 * Replace Fixing nodes with Constant if observed in Market
 */
class ApplyMarket final : public ObservableVisitor<ObservableNodePtr> {
   public:
    explicit ApplyMarket(const market::Market& market) : _market(market) {}
    ~ApplyMarket() override = default;

   protected:
    // Fixing: replace with Constant if observed, otherwise keep as Fixing
    ObservableNodePtr visit(const Fixing& node) override {
        const auto spot = _market.getPrice(node.getSymbol(), node.getDate());

        if (spot.has_value()) {
            return std::make_shared<Constant>(spot.value());
        }

        return std::make_shared<Fixing>(node.getSymbol(), node.getDate());
    }

    ObservableNodePtr visit(const Constant& node) override {
        return std::make_shared<Constant>(node.getValue());
    }

    ObservableNodePtr visit(const Add& node) override {
        return std::make_shared<Add>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    // TODO define template helper function
    ObservableNodePtr visit(const Sum& node) override {
        std::vector<ObservableNodePtr> elements{};
        elements.reserve(node.size());
        for (const auto& element : node) {
            elements.push_back(evaluate(element));
        }
        return std::make_shared<Sum>(std::move(elements));
    }

    ObservableNodePtr visit(const Multiply& node) override {
        return std::make_shared<Multiply>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    ObservableNodePtr visit(const Divide& node) override {
        return std::make_shared<Divide>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    ObservableNodePtr visit(const Max& node) override {
        std::vector<ObservableNodePtr> elements{};
        elements.reserve(node.size());
        for (const auto& element : node) {
            elements.push_back(evaluate(element));
        }
        return std::make_shared<Max>(std::move(elements));
    }

    ObservableNodePtr visit(const Min& node) override {
        std::vector<ObservableNodePtr> elements{};
        elements.reserve(node.size());
        for (const auto& element : node) {
            elements.push_back(evaluate(element));
        }
        return std::make_shared<Min>(std::move(elements));
    }

    ObservableNodePtr visit(const GreaterThan& node) override {
        return std::make_shared<GreaterThan>(evaluate(node.getLeft()), evaluate(node.getRight()));
    }

    ObservableNodePtr visit(const GreaterThanOrEqual& node) override {
        return std::make_shared<GreaterThanOrEqual>(evaluate(node.getLeft()),
                                                    evaluate(node.getRight()));
    }

    ObservableNodePtr visit(const IfThenElse& node) override {
        return std::make_shared<IfThenElse>(evaluate(node.getCond()), evaluate(node.getThen()),
                                            evaluate(node.getElse()));
    }

   private:
    const market::Market& _market;
};

class PayoffApplyMarket final : public PayoffVisitor<PayoffNodePtr> {
   public:
    explicit PayoffApplyMarket(const market::Market& market) : _market(market) {}
    ~PayoffApplyMarket() override = default;

   protected:
    PayoffNodePtr visit(const CashPayment& node) override {
        const auto amount = applyMarket(node.getAmount(), _market);
        return std::make_shared<CashPayment>(amount, node.getSettlementDate());
    }
    PayoffNodePtr visit(const CombinedPayment& node) override {
        const auto left = evaluate(node.getLeft());
        const auto right = evaluate(node.getRight());
        return std::make_shared<CombinedPayment>(left, right);
    }
    PayoffNodePtr visit(const BranchPayment& node) override {
        const auto condition = applyMarket(node.getCondition(), _market);
        const auto then_ = evaluate(node.getThenPayoff());
        const auto else_ = evaluate(node.getElsePayoff());
        return std::make_shared<BranchPayment>(condition, then_, else_);
    }

   private:
    const market::Market& _market;
};
}  // namespace

ObservableNodePtr applyMarket(const ObservableNodePtr& observable, const market::Market& market) {
    return foldConstants(ApplyMarket(market).evaluate(observable));
}
ObservableNodePtr applyMarket(const ObservableNode& observable, const market::Market& market) {
    return foldConstants(ApplyMarket(market).evaluate(observable));
}
PayoffNodePtr applyMarket(const PayoffNodePtr& payoff, const market::Market& market) {
    return PayoffApplyMarket(market).evaluate(payoff);
}

PayoffNodePtr applyMarket(const PayoffNode& payoff, const market::Market& market) {
    return PayoffApplyMarket(market).evaluate(payoff);
}

CashPayment applyMarket(const CashPayment& payoff, const market::Market& market) {
    return {applyMarket(payoff.getAmount(), market), payoff.getSettlementDate()};
}
}  // namespace payoff
