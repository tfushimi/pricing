#pragma once

#include "PayoffNode.h"
#include "numerics/PiecewiseLinearFunction.h"

namespace payoff {

using PL = numerics::pwl::PiecewiseLinearFunction;

class PLVisitor final : public PayoffVisitor<PL> {
public:

  PL visit(const Fixing& node) override {
    // TODO store fixing date for validation
    return PL::createLinear(1.0, 0.0);
  }

  PL visit(const Constant& node) override {

    return PL::createConstant(node.getValue());
  }

  PL visit(const Sum& node) override {

    return evaluate(node.getLeft()) + evaluate(node.getRight());
  }

  PL visit(const Multiply& node) override {

    return evaluate(node.getLeft()) * evaluate(node.getRight());
  }

  PL visit(const Divide& node) override {

      return evaluate(node.getLeft()) / evaluate(node.getRight());
  }

  PL visit(const Max& node) override {

    return PL::max(evaluate(node.getLeft()), evaluate(node.getRight()));
  }

  PL visit(const Min& node) override {

    return PL::min(evaluate(node.getLeft()), evaluate(node.getRight()));
  }
};
}