#include "payoff/PayoffNode.h"

#include "payoff/PiecewiseLinearFunctionVisitor.h"

#include <gtest/gtest.h>

using namespace payoff;

PiecewiseLinearFunctionVisitor visitor;

const auto S = fixing("SPY", "2026/03/20");

TEST(PayoffNodeTest, ArithmeticTest) {

    const auto sum = constant(1.0) + constant(2.0);
    const auto sumPLF = visitor.evaluate(sum);
    EXPECT_DOUBLE_EQ(sumPLF(-1e+10), 3.0);
    EXPECT_DOUBLE_EQ(sumPLF(0.0), 3.0);
    EXPECT_DOUBLE_EQ(sumPLF(1e+10), 3.0);

    const auto sub = constant(1.0) - constant(2.0);
    const auto subPLF = visitor.evaluate(sub);
    EXPECT_DOUBLE_EQ(subPLF(-1e+10), -1.0);
    EXPECT_DOUBLE_EQ(subPLF(0.0), -1.0);
    EXPECT_DOUBLE_EQ(subPLF(1e+10), -1.0);

    const auto mult = constant(2.0) * constant(3.0);
    const auto multPLF = visitor.evaluate(mult);
    EXPECT_DOUBLE_EQ(multPLF(-1e+10), 6.0);
    EXPECT_DOUBLE_EQ(multPLF(0.0), 6.0);
    EXPECT_DOUBLE_EQ(multPLF(1e+10), 6.0);

    const auto divide = constant(1.0) / constant(2.0);
    const auto dividePLF = visitor.evaluate(divide);
    EXPECT_DOUBLE_EQ(dividePLF(-1e+10), 0.5);
    EXPECT_DOUBLE_EQ(dividePLF(0.0), 0.5);
    EXPECT_DOUBLE_EQ(dividePLF(1e+10), 0.5);
}

TEST(PayoffNodeTest, FixingTest) {

    const auto sum = S + 50;
    EXPECT_DOUBLE_EQ(visitor.evaluate(sum)(30.0), 80.0);

    const auto sub = S - 50;
    EXPECT_DOUBLE_EQ(visitor.evaluate(sub)(80.0), 30.0);

    const auto neg = -S;
    EXPECT_DOUBLE_EQ(visitor.evaluate(neg)(5.0), -5.0);

    const auto mult = S * 3.0;
    EXPECT_DOUBLE_EQ(visitor.evaluate(mult)(4.0), 12.0);

    const auto divide = S / 2.0;
    EXPECT_DOUBLE_EQ(visitor.evaluate(divide)(6.0), 3.0);
}

TEST(PayoffNodeTest, GreaterThanTest) {

    const auto greaterThanK = S > 50.0;
    EXPECT_DOUBLE_EQ(visitor.evaluate(greaterThanK)(40.0), 0.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(greaterThanK)(50.0), 0.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(greaterThanK)(60.0), 1.0);

    const auto greaterThanOrEqualToK = S >= 50.0;
    EXPECT_DOUBLE_EQ(visitor.evaluate(greaterThanOrEqualToK)(40.0), 0.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(greaterThanOrEqualToK)(50.0), 1.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(greaterThanOrEqualToK)(60.0), 1.0);

    const auto one = constant(3.0) > constant(2.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(one)(-1e10), 1.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(one)(0), 1.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(one)(1e10), 1.0);
}

TEST(PayoffNodeTest, IfThenElseTest) {

    const auto K = constant(100.0);

    const auto cond = S > K;
    const auto then_ = S - K;
    const auto call = ite(cond, then_, constant(0.0));

    EXPECT_DOUBLE_EQ(visitor.evaluate(call)(50.0), 0.0) << "call: below strike";
    EXPECT_DOUBLE_EQ(visitor.evaluate(call)(150.0), 50.0) << "call: above strike";
}

TEST(PayoffNodeTest, MaxMinTest) {

    const auto zero = constant(0.0);

    // call
    const auto call = max(S - 100.0, 0.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(call)(50.0), 0.0);
    EXPECT_DOUBLE_EQ(visitor.evaluate(call)(150.0), 50.0);

    // call spread
    const auto capped = min(200, max(S - 100, 10));
    EXPECT_DOUBLE_EQ(visitor.evaluate(capped)(50.0), 10.0) << "floored+capped: floor region";
    EXPECT_DOUBLE_EQ(visitor.evaluate(capped)(150.0), 50.0) << "floored+capped: linear region";
    EXPECT_DOUBLE_EQ(visitor.evaluate(capped)(400.0), 200.0) << "floored+capped: cap region";
}