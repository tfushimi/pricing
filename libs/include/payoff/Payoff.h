#pragma once

#include "Payoff.h"
#include "payoff/Observable.h"

namespace payoff {

class PayoffNode;
class CashPayment;
class CombinedPayment;
class BranchPayment;

class PayoffNodePtr {
   public:
    PayoffNodePtr() = default;

    template <typename T, typename = std::enable_if<std::is_base_of_v<PayoffNode, T>>>
    PayoffNodePtr(std::shared_ptr<T> paymentNode) : _ptr(std::move(paymentNode)) {}

    const PayoffNode& operator*() const { return *_ptr; }
    const PayoffNode* operator->() const { return _ptr.get(); }
    const PayoffNode* get() const { return _ptr.get(); }
    PayoffNodePtr operator+(const PayoffNodePtr& right) const;

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
    virtual T visit(const BranchPayment& node) = 0;
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

    enum class Type { CashPayment, CombinedPayment, BranchPayment };

    virtual Type type() const = 0;
    virtual std::string toString() const = 0;
};

class CashPayment final : public PayoffNode {
   public:
    CashPayment(ObservableNodePtr amount, const Date settlementDate)
        : _amount(std::move(amount)), _settlementDate(settlementDate){};

    static constexpr std::string_view NAME = "CashPayment";
    Type type() const override { return Type::CashPayment; }
    std::string toString() const override { return std::string(NAME); }

    const ObservableNode& getAmount() const { return *_amount; }
    Date getSettlementDate() const { return _settlementDate; };

   private:
    const ObservableNodePtr _amount;
    const Date _settlementDate;
};

class CombinedPayment final : public PayoffNode {
   public:
    CombinedPayment(PayoffNodePtr left, PayoffNodePtr right)
        : _left(std::move(left)), _right(std::move(right)){};

    static constexpr std::string_view NAME = "CombinedPayment";
    Type type() const override { return Type::CombinedPayment; };
    std::string toString() const override { return std::string(NAME); }

    const PayoffNode& getLeft() const { return *_left; }
    const PayoffNode& getRight() const { return *_right; }

   private:
    PayoffNodePtr _left;
    PayoffNodePtr _right;
};

// TODO add unit tests
class BranchPayment final : public PayoffNode {
   public:
    BranchPayment(ObservableNodePtr condition, PayoffNodePtr thenPayoff, PayoffNodePtr elsePayoff)
        : _condition(std::move(condition)),
          _thenPayoff(std::move(thenPayoff)),
          _elsePayoff(std::move(elsePayoff)) {}

    static constexpr std::string_view NAME = "BranchPayment";
    Type type() const override { return Type::BranchPayment; }
    std::string toString() const override { return std::string(NAME); }

    const ObservableNode& getCondition() const { return *_condition; }
    const PayoffNode& getThenPayoff() const { return *_thenPayoff; }
    const PayoffNode& getElsePayoff() const { return *_elsePayoff; }

   private:
    ObservableNodePtr _condition;
    PayoffNodePtr _thenPayoff;
    PayoffNodePtr _elsePayoff;
};

inline PayoffNodePtr cashPayment(ObservableNodePtr amount, const Date& settlementDate) {
    return std::make_shared<CashPayment>(std::move(amount), settlementDate);
}

inline PayoffNodePtr combinedPayment(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<CombinedPayment>(std::move(left), std::move(right));
}

inline PayoffNodePtr branchPayment(ObservableNodePtr condition, PayoffNodePtr left,
                                   PayoffNodePtr right) {
    return std::make_shared<BranchPayment>(std::move(condition), std::move(left), std::move(right));
}

inline PayoffNodePtr PayoffNodePtr::operator+(const PayoffNodePtr& right) const {
    return combinedPayment(*this, right);
}

template <typename T>
T PayoffVisitor<T>::evaluate(const PayoffNode& node) {
    switch (node.type()) {
        case PayoffNode::Type::CashPayment:
            return visit(static_cast<const CashPayment&>(node));
        case PayoffNode::Type::CombinedPayment:
            return visit(static_cast<const CombinedPayment&>(node));
        case PayoffNode::Type::BranchPayment:
            return visit(static_cast<const BranchPayment&>(node));
    }

    throw std::invalid_argument("Unknown payment node type");
}

}  // namespace payoff