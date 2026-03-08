#pragma once

#include "common/types.h"
#include "payoff/Observable.h"

namespace payoff {

class PayoffNode;
class CashPayment;
class CombinedPayment;
class MultiplyPayment;

class PayoffNodePtr {
   public:
    PayoffNodePtr() = default;

    template <typename T, typename = std::enable_if<std::is_base_of_v<PayoffNode, T>>>
    PayoffNodePtr(std::shared_ptr<T> paymentNode) : _ptr(std::move(paymentNode)) {}

    const PayoffNode& operator*() const { return *_ptr; }
    const PayoffNode* operator->() const { return _ptr.get(); }
    const PayoffNode* get() const { return _ptr.get(); }
    // TODO support + and *
   private:
    std::shared_ptr<PayoffNode> _ptr;
};

template <typename T>
class PayoffVisitor {
   public:
    virtual ~PayoffVisitor() = default;

    T evaluate(const PayoffNode& node);
    T evaluate(const PayoffNodePtr& node) { return evaluate(*node); }
    T evaluate(PayoffNodePtr&& node) = delete;

   protected:
    virtual T visit(const CashPayment& node) = 0;
    virtual T visit(const CombinedPayment& node) = 0;
    virtual T visit(const MultiplyPayment& node) = 0;
};

// Base node
class PayoffNode {
   public:
    PayoffNode() = default;
    explicit PayoffNode(const PayoffNodePtr& node) = delete;
    explicit PayoffNode(PayoffNodePtr&& node) = delete;
    PayoffNode& operator=(const PayoffNodePtr& node) = delete;
    PayoffNodePtr& operator=(PayoffNodePtr&& node) = delete;
    virtual ~PayoffNode() = default;

    enum class Type { CashPayment, CombinedPayment, MultiplyPayment };

    virtual Type type() const = 0;
};

class CashPayment final : public PayoffNode {
   public:
    CashPayment(ObservableNodePtr amount, const Date settlementDate)
        : _amount(std::move(amount)), _settlementDate(settlementDate){};

    Type type() const override { return Type::CashPayment; }
    const ObservableNode& getAmount() const { return *_amount; }
    const ObservableNodePtr& getAmountPtr() const { return _amount; };
    Date getSettlementDate() const { return _settlementDate; };

   private:
    const ObservableNodePtr _amount;
    const Date _settlementDate;
};

class CombinedPayment final : public PayoffNode {
   public:
    CombinedPayment(PayoffNodePtr left, PayoffNodePtr right)
        : _left(std::move(left)), _right(std::move(right)){};

    Type type() const override { return Type::CombinedPayment; };

    const PayoffNode& getLeft() const { return *_left; }
    const PayoffNode& getRight() const { return *_right; }
    const PayoffNodePtr& getLeftPtr() const { return _left; }
    const PayoffNodePtr& getRightPtr() const { return _right; }

   private:
    PayoffNodePtr _left;
    PayoffNodePtr _right;
};

class MultiplyPayment final : public PayoffNode {
   public:
    MultiplyPayment(PayoffNodePtr payoff, const double multiplier)
        : _payment(std::move(payoff)), _multiplier(multiplier){};

    Type type() const override { return Type::MultiplyPayment; }

    const PayoffNode& getPayoff() const { return *_payment; }
    const PayoffNodePtr& getPaymentPtr() const { return _payment; };
    double multiplier() const { return _multiplier; }

   private:
    PayoffNodePtr _payment;
    double _multiplier;
};

inline PayoffNodePtr cashPayment(ObservableNodePtr amount, const Date& settlementDate) {
    return std::make_shared<CashPayment>(std::move(amount), settlementDate);
}

inline PayoffNodePtr combinedPayment(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<CombinedPayment>(std::move(left), std::move(right));
}

inline PayoffNodePtr multiplyPayment(PayoffNodePtr payment, const double multiplier) {
    return std::make_shared<MultiplyPayment>(std::move(payment), multiplier);
}

template <typename T>
T PayoffVisitor<T>::evaluate(const PayoffNode& node) {
    switch (node.type()) {
        case PayoffNode::Type::CashPayment:
            return visit(static_cast<const CashPayment&>(node));
        case PayoffNode::Type::CombinedPayment:
            return visit(static_cast<const CombinedPayment&>(node));
        case PayoffNode::Type::MultiplyPayment:
            return visit(static_cast<const MultiplyPayment&>(node));
    }

    throw std::invalid_argument("Unknown payment node type");
}

}  // namespace payoff