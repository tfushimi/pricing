#pragma once

#include <chrono>
#include <memory>
#include <stdexcept>

#include "common/Date.h"

namespace payoff {

using calendar::Date;

class ObservableNode;
class Fixing;
class Constant;
class Add;
class Sum;
class Multiply;
class Divide;
class Max;
class Min;
class GreaterThan;
class GreaterThanOrEqual;
class IfThenElse;

/**
 * Payoff DSL is a DAG of ObservableNodePtr.
 *
 * Directed = child ObservableNodes hold no pointers back to their parents,
 *            edges flow strictly from parent to child.
 *
 * Acyclic = ObservableNodes are immutable after construction (no setters exposed),
 *           and a parent node can only reference already-constructed children.
 *           Therefore, no node can be its own ancestor.
 *
 * The DSL factory methods copy shared_ptr, which increments ref counts and ensures shared ownership
 * of the underlying ObservableNode.
 */
class ObservableNodePtr {
   public:
    ObservableNodePtr() = default;

    // implicit conversion from shared_ptr
    template <typename T, typename = std::enable_if<std::is_base_of_v<ObservableNode, T>>>
    ObservableNodePtr(std::shared_ptr<T> payoffNode) : _ptr(std::move(payoffNode)) {}

    // implicit conversion from double
    ObservableNodePtr(double value);

    // Pointer methods
    const ObservableNode& operator*() const { return *_ptr; }
    const ObservableNode* operator->() const { return _ptr.get(); }
    [[nodiscard]] const ObservableNode* get() const { return _ptr.get(); }
    explicit operator bool() const { return _ptr.get() != nullptr; }

    // Arithmetic operators
    ObservableNodePtr operator+(const ObservableNodePtr& other) const;
    ObservableNodePtr operator-(const ObservableNodePtr& other) const;
    ObservableNodePtr operator*(const ObservableNodePtr& other) const;
    ObservableNodePtr operator/(const ObservableNodePtr& other) const;
    ObservableNodePtr operator-() const;
    ObservableNodePtr operator>(const ObservableNodePtr& other) const;
    ObservableNodePtr operator>=(const ObservableNodePtr& other) const;

   private:
    std::shared_ptr<ObservableNode> _ptr;
};

/**
 * ObservableVisitor traverses a Payoff DSL tree, and subclasses can perform custom operations for
 * each node type.
 */
template <typename T>
class ObservableVisitor {
   public:
    virtual ~ObservableVisitor() = default;

    T evaluate(const ObservableNode& node);
    T evaluate(const ObservableNodePtr& node) { return evaluate(*node); }

    // Delete rvalue overload to catch bugs at compile time
    T evaluate(ObservableNodePtr&&) = delete;

   protected:
    virtual T visit(const Fixing& node) = 0;
    virtual T visit(const Constant& node) = 0;
    virtual T visit(const Add& node) = 0;
    virtual T visit(const Sum& node) = 0;
    virtual T visit(const Multiply& node) = 0;
    virtual T visit(const Divide& node) = 0;
    virtual T visit(const Max& node) = 0;
    virtual T visit(const Min& node) = 0;
    virtual T visit(const GreaterThan& node) = 0;
    virtual T visit(const GreaterThanOrEqual& node) = 0;
    virtual T visit(const IfThenElse& node) = 0;
};

/**
 * All subclasses of ObservableNode must be immutable to ensure the correctness of shared ownership.
 */
class ObservableNode {
   public:
    ObservableNode() = default;
    explicit ObservableNode(const ObservableNodePtr& node) = delete;
    explicit ObservableNode(ObservableNodePtr&& node) = delete;
    ObservableNode& operator=(const ObservableNodePtr& node) = delete;
    ObservableNodePtr& operator=(ObservableNodePtr&& node) = delete;
    virtual ~ObservableNode() = default;

    enum class Type {
        Fixing,
        Constant,
        Add,
        Sum,
        Multiply,
        Divide,
        Max,
        Min,
        GreaterThan,
        GreaterThanOrEqual,
        IfThenElse
    };

    virtual Type type() const = 0;

    virtual std::string toString() const = 0;
};

// TODO support FixingType such as CLOSE (i.e., close price)
class Fixing final : public ObservableNode {
   public:
    explicit Fixing(std::string symbol, Date date)
        : _symbol(std::move(symbol)), _date(std::move(date)) {}
    const std::string& getSymbol() const { return _symbol; }
    const Date& getDate() const { return _date; }
    Type type() const override { return Type::Fixing; }
    std::string toString() const override { return "Fixing"; }

    // define < and == to be comparable for std::set
    bool operator<(const Fixing& other) const {
        if (_symbol != other._symbol) {
            return _symbol < other._symbol;
        }
        return _date < other._date;
    }

    bool operator==(const Fixing& other) const {
        return _symbol == other._symbol && _date == other._date;
    }

   private:
    std::string _symbol;
    Date _date;
};

class Constant final : public ObservableNode {
   public:
    explicit Constant(const double value) : _value(value) {}
    double getValue() const { return _value; }
    Type type() const override { return Type::Constant; }
    std::string toString() const override { return "Constant"; }

   private:
    double _value;
};

class BinaryNode : public ObservableNode {
   public:
    explicit BinaryNode(ObservableNodePtr left, ObservableNodePtr right)
        : _left(std::move(left)), _right(std::move(right)) {}
    const ObservableNode& getLeft() const { return *_left; }
    const ObservableNode& getRight() const { return *_right; }

   private:
    ObservableNodePtr _left;
    ObservableNodePtr _right;
};

class Add final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Add; }
    std::string toString() const override { return "Add"; }
};

class Multiply final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Multiply; }
    std::string toString() const override { return "Multiply"; }
};

class Divide final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::Divide; }
    std::string toString() const override { return "Divide"; }
};

class GreaterThan final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::GreaterThan; }
    std::string toString() const override { return "GreaterThan"; }
};

class GreaterThanOrEqual final : public BinaryNode {
   public:
    using BinaryNode::BinaryNode;
    Type type() const override { return Type::GreaterThanOrEqual; }
    std::string toString() const override { return "GreaterThanOrEqual"; }
};

class VectorNode : public ObservableNode {
   public:
    explicit VectorNode(std::vector<ObservableNodePtr> operands) : _operands(std::move(operands)) {}
    using const_iterator = std::vector<ObservableNodePtr>::const_iterator;
    const_iterator begin() const { return _operands.begin(); }
    const_iterator end() const { return _operands.end(); }
    size_t size() const { return _operands.size(); }

   private:
    std::vector<ObservableNodePtr> _operands{};
};

class Max final : public VectorNode {
   public:
    using VectorNode::VectorNode;
    Type type() const override { return Type::Max; }
    std::string toString() const override { return "Max"; }
};

class Min final : public VectorNode {
   public:
    using VectorNode::VectorNode;
    Type type() const override { return Type::Min; }
    std::string toString() const override { return "Min"; }
};

class Sum final : public VectorNode {
   public:
    using VectorNode::VectorNode;
    Type type() const override { return Type::Sum; }
    std::string toString() const override { return "Sum"; }
};

class IfThenElse final : public ObservableNode {
   public:
    explicit IfThenElse(ObservableNodePtr cond, ObservableNodePtr then_, ObservableNodePtr else_)
        : _cond(std::move(cond)), _then(std::move(then_)), _else(std::move(else_)) {}
    const ObservableNode& getCond() const { return *_cond; }
    const ObservableNode& getThen() const { return *_then; }
    const ObservableNode& getElse() const { return *_else; }
    Type type() const override { return Type::IfThenElse; }
    std::string toString() const override { return "IfThenElse"; }

   private:
    ObservableNodePtr _cond;
    ObservableNodePtr _then;
    ObservableNodePtr _else;
};

/**
 * DSL factory methods should take ObservableNodePtr by value to copy the underlying shared_ptr.
 */
inline ObservableNodePtr fixing(std::string symbol, Date date) {
    return std::make_shared<Fixing>(std::move(symbol), std::move(date));
}

inline ObservableNodePtr constant(double value) {
    return std::make_shared<Constant>(value);
}

inline ObservableNodePtr add(ObservableNodePtr left, ObservableNodePtr right) {
    return std::make_shared<Add>(std::move(left), std::move(right));
}

template <typename... Args>
ObservableNodePtr sum(ObservableNodePtr first, Args&&... rest) {
    std::vector<ObservableNodePtr> nodes;
    nodes.reserve(1 + sizeof...(rest));
    nodes.push_back(std::move(first));
    (nodes.push_back(std::forward<Args>(rest)), ...);
    return std::make_shared<Sum>(std::move(nodes));
}

inline ObservableNodePtr sum(std::vector<ObservableNodePtr> nodes) {
    return std::make_shared<Sum>(std::move(nodes));
}

inline ObservableNodePtr multiply(ObservableNodePtr left, ObservableNodePtr right) {
    return std::make_shared<Multiply>(std::move(left), std::move(right));
}

inline ObservableNodePtr divide(ObservableNodePtr left, ObservableNodePtr right) {
    return std::make_shared<Divide>(std::move(left), std::move(right));
}

inline ObservableNodePtr sub(ObservableNodePtr left, ObservableNodePtr right) {
    return std::make_shared<Add>(std::move(left), multiply(constant(-1.0), std::move(right)));
}

template <typename... Args>
ObservableNodePtr max(ObservableNodePtr first, Args&&... rest) {
    std::vector<ObservableNodePtr> nodes;
    nodes.reserve(1 + sizeof...(rest));
    nodes.push_back(std::move(first));
    (nodes.push_back(std::forward<Args>(rest)), ...);
    return std::make_shared<Max>(std::move(nodes));
}

inline ObservableNodePtr max(std::vector<ObservableNodePtr> nodes) {
    return std::make_shared<Max>(std::move(nodes));
}

template <typename... Args>
ObservableNodePtr min(ObservableNodePtr first, Args&&... rest) {
    std::vector<ObservableNodePtr> nodes;
    nodes.reserve(1 + sizeof...(rest));
    nodes.push_back(std::move(first));
    (nodes.push_back(std::forward<Args>(rest)), ...);
    return std::make_shared<Min>(std::move(nodes));
}

inline ObservableNodePtr min(std::vector<ObservableNodePtr> nodes) {
    return std::make_shared<Min>(std::move(nodes));
}

inline ObservableNodePtr greaterThan(ObservableNodePtr left, ObservableNodePtr right) {
    return std::make_shared<GreaterThan>(std::move(left), std::move(right));
}

inline ObservableNodePtr greaterThanOrEqual(ObservableNodePtr left, ObservableNodePtr right) {
    return std::make_shared<GreaterThanOrEqual>(std::move(left), std::move(right));
}

// TODO support ternary operator?
inline ObservableNodePtr ite(ObservableNodePtr cond, ObservableNodePtr then_,
                             ObservableNodePtr else_) {
    return std::make_shared<IfThenElse>(std::move(cond), std::move(then_), std::move(else_));
}

/**
 * ObservableNodePtr's arithmetic operators should call the factory method to ensure the shared
 * ownership of ObservableNode.
 */
inline ObservableNodePtr::ObservableNodePtr(double value)
    : _ptr(std::make_shared<Constant>(value)) {}

inline ObservableNodePtr ObservableNodePtr::operator+(const ObservableNodePtr& other) const {
    return add(*this, other);
}

inline ObservableNodePtr ObservableNodePtr::operator-(const ObservableNodePtr& other) const {
    return sub(*this, other);
}

inline ObservableNodePtr ObservableNodePtr::operator*(const ObservableNodePtr& other) const {
    return multiply(*this, other);
}

inline ObservableNodePtr ObservableNodePtr::operator/(const ObservableNodePtr& other) const {
    return divide(*this, other);
}

inline ObservableNodePtr ObservableNodePtr::operator-() const {
    return multiply(*this, -1.0);
}

inline ObservableNodePtr ObservableNodePtr::operator>(const ObservableNodePtr& other) const {
    return greaterThan(*this, other);
}

inline ObservableNodePtr ObservableNodePtr::operator>=(const ObservableNodePtr& other) const {
    return greaterThanOrEqual(*this, other);
}

// arithmetic operators for double left
inline ObservableNodePtr operator+(const double left, const ObservableNodePtr& right) {
    return add(left, right);
}
inline ObservableNodePtr operator-(const double left, const ObservableNodePtr& right) {
    return sub(left, right);
}
inline ObservableNodePtr operator*(const double left, const ObservableNodePtr& right) {
    return multiply(left, right);
}
inline ObservableNodePtr operator/(const double left, const ObservableNodePtr& right) {
    return divide(left, right);
}

// compound assignment operators
inline ObservableNodePtr& operator+=(ObservableNodePtr& lhs, const ObservableNodePtr& rhs) {
    lhs = lhs + rhs;
    return lhs;
}

inline ObservableNodePtr& operator-=(ObservableNodePtr& lhs, const ObservableNodePtr& rhs) {
    lhs = lhs - rhs;
    return lhs;
}

template <typename T>
T ObservableVisitor<T>::evaluate(const ObservableNode& node) {
    switch (node.type()) {
        case ObservableNode::Type::Fixing:
            return visit(static_cast<const Fixing&>(node));
        case ObservableNode::Type::Constant:
            return visit(static_cast<const Constant&>(node));
        case ObservableNode::Type::Add:
            return visit(static_cast<const Add&>(node));
        case ObservableNode::Type::Sum:
            return visit(static_cast<const Sum&>(node));
        case ObservableNode::Type::Multiply:
            return visit(static_cast<const Multiply&>(node));
        case ObservableNode::Type::Divide:
            return visit(static_cast<const Divide&>(node));
        case ObservableNode::Type::Max:
            return visit(static_cast<const Max&>(node));
        case ObservableNode::Type::Min:
            return visit(static_cast<const Min&>(node));
        case ObservableNode::Type::GreaterThan:
            return visit(static_cast<const GreaterThan&>(node));
        case ObservableNode::Type::GreaterThanOrEqual:
            return visit(static_cast<const GreaterThanOrEqual&>(node));
        case ObservableNode::Type::IfThenElse:
            return visit(static_cast<const IfThenElse&>(node));
    }

    throw std::invalid_argument("Unknown payoff node type");
}
}  // namespace payoff