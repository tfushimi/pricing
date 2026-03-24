#pragma once

#include "payoff/Observable.h"
#include "payoff/Payoff.h"

using namespace payoff;

template <typename T>
const T* asNode(const ObservableNodePtr& node) {
    return dynamic_cast<const T*>(node.get());
}

template <typename T>
const T* asNode(const ObservableNode& node) {
    return dynamic_cast<const T*>(&node);
}

template <typename T>
const T* asNode(const PayoffNodePtr& node) {
    return dynamic_cast<const T*>(node.get());
}

template <typename T>
const T* asNode(const PayoffNode& node) {
    return dynamic_cast<const T*>(&node);
}
