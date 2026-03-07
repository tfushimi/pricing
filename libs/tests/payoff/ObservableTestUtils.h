#pragma once

#include "payoff/Observable.h"
using namespace payoff;

inline const Constant* asConstant(const ObservableNodePtr& node) {
    return dynamic_cast<const Constant*>(node.get());
}

inline const Fixing* asFixing(const ObservableNodePtr& node) {
    return dynamic_cast<const Fixing*>(node.get());
}