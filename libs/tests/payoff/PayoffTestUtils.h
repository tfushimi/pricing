#pragma once

#include "payoff/PayoffNode.h"
using namespace payoff;

inline const Constant* asConstant(const PayoffNodePtr& node) {
    return dynamic_cast<const Constant*>(node.get());
}

inline const Fixing* asFixing(const PayoffNodePtr& node) {
    return dynamic_cast<const Fixing*>(node.get());
}