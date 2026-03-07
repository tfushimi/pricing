#pragma once

#include "Payment.h"
#include "common/types.h"
#include "payoff/PayoffNode.h"

namespace payoff {

class PaymentNode;
class CashPayment;
class CombinedPayment;
class MultiPayment;

class PaymentNodePtr {
   public:
    PaymentNodePtr() = default;

    template <typename T, typename = std::enable_if<std::is_base_of_v<PaymentNode, T>>>
    PaymentNodePtr(std::shared_ptr<T> paymentNode) : _ptr(std::move(paymentNode)) {}

    const PaymentNode& operator*() const { return *_ptr; }
    const PaymentNode* operator->() const { return _ptr.get(); }
    const PaymentNode* get() const { return _ptr.get(); }
    // TODO support + and *
   private:
    std::shared_ptr<PaymentNode> _ptr;
};

template <typename T>
class PaymentVisitor {
   public:
    virtual ~PaymentVisitor() = default;

    T evaluate(const PaymentNode& node);
    T evaluate(const PaymentNodePtr& node) { return evaluate(*node); }
    T evaluate(PaymentNodePtr&& node) = delete;

   protected:
    virtual T visit(const CashPayment& node) = 0;
    virtual T visit(const CombinedPayment& node) = 0;
    virtual T visit(const MultiPayment& node) = 0;
};

// Base node
class PaymentNode {
   public:
    PaymentNode() = default;
    explicit PaymentNode(const PaymentNodePtr& node) = delete;
    explicit PaymentNode(PaymentNodePtr&& node) = delete;
    PaymentNode& operator=(const PaymentNodePtr& node) = delete;
    PaymentNodePtr& operator=(PaymentNodePtr&& node) = delete;
    virtual ~PaymentNode() = default;

    enum class Type { CashPayment, CombinedPayment, MultiplyPayment };

    virtual Type type() const = 0;
};

class CashPayment final : public PaymentNode {
   public:
    CashPayment(PayoffNodePtr amount, const Date settlementDate)
        : _amount(std::move(amount)), _settlementDate(settlementDate){};

    Type type() const override { return Type::CashPayment; }
    const PayoffNodePtr& getAmount() const { return _amount; };
    Date getSettlementDate() const { return _settlementDate; };

   private:
    const PayoffNodePtr _amount;
    const Date _settlementDate;
};

class CombinedPayment final : public PaymentNode {
   public:
    CombinedPayment(PaymentNodePtr left, PaymentNodePtr right)
        : _left(std::move(left)), _right(std::move(right)){};

    Type type() const override { return Type::CombinedPayment; };

    const PaymentNode& getLeft() const { return *_left; }
    const PaymentNode& getRight() const { return *_right; }
    const PaymentNodePtr& getLeftPtr() const { return _left; }
    const PaymentNodePtr& getRightPtr() const { return _right; }

   private:
    PaymentNodePtr _left;
    PaymentNodePtr _right;
};

class MultiPayment final : public PaymentNode {
   public:
    MultiPayment(PaymentNodePtr payment, const double multiplier)
        : _payment(std::move(payment)), _multiplier(multiplier){};

    Type type() const override { return Type::MultiplyPayment; }

    const PaymentNode& getPayment() const { return *_payment; }
    const PaymentNodePtr& getPaymentPtr() const { return _payment; };
    double multiplier() const { return _multiplier; }

   private:
    PaymentNodePtr _payment;
    double _multiplier;
};

inline PaymentNodePtr cashPayment(PayoffNodePtr amount, const Date& settlementDate) {
    return std::make_shared<CashPayment>(std::move(amount), settlementDate);
}

inline PaymentNodePtr combinedPayment(PaymentNodePtr left, PaymentNodePtr right) {
    return std::make_shared<CombinedPayment>(std::move(left), std::move(right));
}

template <typename T>
T PaymentVisitor<T>::evaluate(const PaymentNode& node) {
    switch (node.type()) {
        case PaymentNode::Type::CashPayment:
            return visit(static_cast<const PaymentNode&>(node));
        case PaymentNode::Type::CombinedPayment:
            return visit(static_cast<const PaymentNode&>(node));
        case PaymentNode::Type::MultiplyPayment:
            return visit(static_cast<const PaymentNode&>(node));
    }

    throw std::invalid_argument("Unknown payment node type");
}

}  // namespace payoff