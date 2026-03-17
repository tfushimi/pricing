#pragma once

#include "payoff/Observable.h"
#include "payoff/Payoff.h"

using namespace payoff;

inline const Constant* asConstant(const ObservableNodePtr& node) {
    return dynamic_cast<const Constant*>(node.get());
}
inline const Constant* asConstant(const ObservableNode& node) {
    return dynamic_cast<const Constant*>(&node);
}

inline const Fixing* asFixing(const ObservableNodePtr& node) {
    return dynamic_cast<const Fixing*>(node.get());
}
inline const Fixing* asFixing(const ObservableNode& node) {
    return dynamic_cast<const Fixing*>(&node);
}

inline const CashPayment* asCashPayment(const PayoffNodePtr& node) {
    return dynamic_cast<const CashPayment*>(node.get());
}
inline const CashPayment* asCashPayment(const PayoffNode& node) {
    return dynamic_cast<const CashPayment*>(&node);
}

inline const CombinedPayment* asCombinedPayment(const PayoffNodePtr& node) {
    return dynamic_cast<const CombinedPayment*>(node.get());
}
inline const CombinedPayment* asCombinedPayment(const PayoffNode& node) {
    return dynamic_cast<const CombinedPayment*>(&node);
}