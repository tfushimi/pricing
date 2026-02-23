#pragma once

#include <memory>
#include <stdexcept>

#include "FixingDate.h"
#include "PayoffNode.h"

namespace payoff {

class PayoffNode;
class Fixing;
class Constant;
class Sum;
class Multiply;
class Divide;
class Max;
class Min;
class GreaterThan;
class GreaterThanOrEqual;
// class IfThenElse;

class PayoffNodePtr {
   public:
    PayoffNodePtr() = default;

    // implicit conversion from shared_ptr
    template <typename T, typename = std::enable_if<std::is_base_of_v<PayoffNode, T>>>
    PayoffNodePtr(std::shared_ptr<T> payoffNode) : _ptr(std::move(payoffNode)) {}

    // implicit conversion from double
    PayoffNodePtr(double value);

    // Pointer methods
    const PayoffNode& operator*() const { return *_ptr; }
    const PayoffNode* operator->() const { return _ptr.get(); }
    const std::shared_ptr<const PayoffNode>& ptr() const { return _ptr; }
    explicit operator bool() const { return _ptr.get() != nullptr; }

    // Arithmetic operators
    PayoffNodePtr operator+(const PayoffNodePtr& other) const;
    PayoffNodePtr operator-(const PayoffNodePtr& other) const;
    PayoffNodePtr operator*(const PayoffNodePtr& other) const;
    PayoffNodePtr operator/(const PayoffNodePtr& other) const;
    PayoffNodePtr operator-() const;
    PayoffNodePtr operator>(const PayoffNodePtr& other) const;
    PayoffNodePtr operator>=(const PayoffNodePtr& other) const;

   private:
    std::shared_ptr<PayoffNode> _ptr;
};

// arithmetic operators for double hls
inline PayoffNodePtr operator+(double lhs, const PayoffNodePtr& rhs) {
    return PayoffNodePtr(lhs) + rhs;
}
inline PayoffNodePtr operator-(double lhs, const PayoffNodePtr& rhs) {
    return PayoffNodePtr(lhs) - rhs;
}
inline PayoffNodePtr operator*(double lhs, const PayoffNodePtr& rhs) {
    return PayoffNodePtr(lhs) * rhs;
}

// Visitor
template <typename T>
class PayoffVisitor {
   public:
    virtual ~PayoffVisitor() = default;
    virtual T visit(const Fixing& node) = 0;
    virtual T visit(const Constant& node) = 0;
    virtual T visit(const Sum& node) = 0;
    virtual T visit(const Multiply& node) = 0;
    virtual T visit(const Divide& node) = 0;
    virtual T visit(const Max& node) = 0;
    virtual T visit(const Min& node) = 0;
    virtual T visit(const GreaterThan& node)=0;
    virtual T visit(const GreaterThanOrEqual& node)=0;
    // virtual T visit(const IfThenElse& node)=0;

    T evaluate(const PayoffNode& node);
    T evaluate(const PayoffNodePtr& node) { return evaluate(*node); }

    // Delete rvalue overload to catch bugs at compile time
    T evaluate(PayoffNodePtr&&) = delete;
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

    enum class Type { Fixing, Constant, Sum, Multiply, Divide, Max, Min, GreaterThan, GreaterThanOrEqual };

    virtual Type type() const = 0;
};

class Fixing final : public PayoffNode {
   public:
    explicit Fixing(FixingDate date) : _date(std::move(date)) {}
    const FixingDate& getDate() const { return _date; }
    Type type() const override { return Type::Fixing; }

   private:
    FixingDate _date;
};

class Constant final : public PayoffNode {
   public:
    explicit Constant(double value) : _value(value) {}
    double getValue() const { return _value; }
    Type type() const override { return Type::Constant; }

   private:
    double _value;
};

class BinaryNode : public PayoffNode {
   public:
    explicit BinaryNode(PayoffNodePtr left, PayoffNodePtr right)
        : _left(std::move(left)), _right(std::move(right)) {}
    const PayoffNode& getLeft() const { return *_left; }
    const PayoffNode& getRight() const { return *_right; }
    const PayoffNodePtr& getLeftPtr() const { return _left; }
    const PayoffNodePtr& getRightPtr() const { return _right; }

   private:
    PayoffNodePtr _left;
    PayoffNodePtr _right;
};

class Sum final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Sum; }
};

class Multiply final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Multiply; }
};

class Divide final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Divide; }
};

class Max final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Max; }
};

class Min final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Min; }
};

class GreaterThan final : public BinaryNode {
    public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::GreaterThan; }
};

class GreaterThanOrEqual final : public BinaryNode {
    public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::GreaterThanOrEqual; }
};

// Factory functions
inline PayoffNodePtr fixing(FixingDate date) {
    return std::make_shared<Fixing>(std::move(date));
}

inline PayoffNodePtr constant(double value) {
    return std::make_shared<Constant>(value);
}

inline PayoffNodePtr add(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<Sum>(std::move(left), std::move(right));
}

inline PayoffNodePtr multiply(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<Multiply>(std::move(left), std::move(right));
}

inline PayoffNodePtr divide(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<Divide>(std::move(left), std::move(right));
}

inline PayoffNodePtr sub(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<Sum>(std::move(left), multiply(constant(-1.0), std::move(right)));
}

inline PayoffNodePtr max(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<Max>(std::move(left), std::move(right));
}

inline PayoffNodePtr min(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<Min>(std::move(left), std::move(right));
}

inline PayoffNodePtr greaterThan(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<GreaterThan>(std::move(left), std::move(right));
}

inline PayoffNodePtr greaterThanOrEqual(PayoffNodePtr left, PayoffNodePtr right) {
    return std::make_shared<GreaterThanOrEqual>(std::move(left), std::move(right));
}

// PayoffNodePtr operator implementations
inline PayoffNodePtr::PayoffNodePtr(double value) : _ptr(std::make_shared<Constant>(value)) {}

inline PayoffNodePtr PayoffNodePtr::operator+(const PayoffNodePtr& other) const {
    return add(*this, other);
}

inline PayoffNodePtr PayoffNodePtr::operator-(const PayoffNodePtr& other) const {
    return add(*this, -other);
}

inline PayoffNodePtr PayoffNodePtr::operator*(const PayoffNodePtr& other) const {
    return multiply(*this, other);
}

inline PayoffNodePtr PayoffNodePtr::operator/(const PayoffNodePtr& other) const {
    return divide(*this, other);
}

inline PayoffNodePtr PayoffNodePtr::operator-() const {
    return multiply(*this, -1.0);
}

inline PayoffNodePtr PayoffNodePtr::operator>(const PayoffNodePtr& other) const {
    return greaterThan(*this, other);
}

inline PayoffNodePtr PayoffNodePtr::operator>=(const PayoffNodePtr& other) const {
    return greaterThanOrEqual(*this, other);
}

template <typename T>
T PayoffVisitor<T>::evaluate(const PayoffNode& node) {
    switch (node.type()) {
        case PayoffNode::Type::Fixing:
            return visit(static_cast<const Fixing&>(node));
        case PayoffNode::Type::Constant:
            return visit(static_cast<const Constant&>(node));
        case PayoffNode::Type::Sum:
            return visit(static_cast<const Sum&>(node));
        case PayoffNode::Type::Multiply:
            return visit(static_cast<const Multiply&>(node));
        case PayoffNode::Type::Divide:
            return visit(static_cast<const Divide&>(node));
        case PayoffNode::Type::Max:
            return visit(static_cast<const Max&>(node));
        case PayoffNode::Type::Min:
            return visit(static_cast<const Min&>(node));
        case PayoffNode::Type::GreaterThan:
            return visit(static_cast<const GreaterThan&>(node));
        case PayoffNode::Type::GreaterThanOrEqual:
            return visit(static_cast<const GreaterThanOrEqual&>(node));
    }

    throw std::invalid_argument("Unknown payoff node type");
}
}  // namespace payoff