#include "payoff/Transforms.h"

#include <gtest/gtest.h>

#include "payoff/PayoffNode.h"

using namespace payoff;
using namespace payoff;

const Constant* asConstant(const PayoffNodePtr& node) {
    return dynamic_cast<const Constant*>(node.get());
}

const Fixing* asFixing(const PayoffNodePtr& node) {
    return dynamic_cast<const Fixing*>(node.get());
}

const auto spy = fixing("SPY", makeDate(2026, 3, 20));

TEST(ConstantFoldVisitorTest, ArithmeticTest) {
    const auto sum = constant(1.0) + constant(2.0);
    const auto* sumConstant = asConstant(foldConstants(sum));
    ASSERT_NE(sumConstant, nullptr);
    EXPECT_DOUBLE_EQ(sumConstant->getValue(), 3.0);

    const auto sub = constant(1.0) - constant(2.0);
    const auto* subConstant = asConstant(foldConstants(sub));
    ASSERT_NE(subConstant, nullptr);
    EXPECT_DOUBLE_EQ(subConstant->getValue(), -1.0);

    const auto mult = constant(2.0) * constant(3.0);
    const auto* multConstant = asConstant(foldConstants(mult));
    ASSERT_NE(multConstant, nullptr);
    EXPECT_DOUBLE_EQ(multConstant->getValue(), 6.0);

    const auto divide = constant(1.0) / constant(2.0);
    const auto* divideConstant = asConstant(foldConstants(divide));
    ASSERT_NE(divideConstant, nullptr);
    EXPECT_DOUBLE_EQ(divideConstant->getValue(), 0.5);
}

TEST(ConstantFoldVisitorTest, MultiplyTest) {
    const auto zero = constant(0.0);
    const auto one = constant(1.0);

    {
        const auto mult = spy * zero;
        const auto* multConstant = asConstant(foldConstants(mult));
        ASSERT_NE(multConstant, nullptr);
        EXPECT_DOUBLE_EQ(multConstant->getValue(), 0.0);
    }

    {
        const auto mult = zero * spy;
        const auto* multConstant = asConstant(foldConstants(mult));
        ASSERT_NE(multConstant, nullptr);
        EXPECT_DOUBLE_EQ(multConstant->getValue(), 0.0);
    }

    {
        const auto mult = spy * one;
        const auto* fixing = asFixing(foldConstants(mult));
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }

    {
        const auto mult = one * spy;
        const auto* fixing = asFixing(foldConstants(mult));
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }
}

TEST(ConstantFoldVisitorTest, DivideTest) {
    const auto divide = spy / constant(1.0);
    const auto* divideConstant = asFixing(foldConstants(divide));
    ASSERT_NE(divideConstant, nullptr);
    EXPECT_EQ(divideConstant->getSymbol(), "SPY");
}

TEST(ConstantFoldVisitorTest, DivideByZeroThrows) {
    const auto divide = constant(1.0) / constant(0.0);
    EXPECT_THROW(foldConstants(divide), std::invalid_argument);
}

TEST(ConstantFoldVisitorTest, MaxMinTest) {
    const auto maxPayoff = max(constant(1.0), constant(2.0));
    const auto* maxConstant = asConstant(foldConstants(maxPayoff));
    ASSERT_NE(maxConstant, nullptr);
    EXPECT_DOUBLE_EQ(maxConstant->getValue(), 2.0);

    const auto minPayoff = min(constant(1.0), constant(2.0));
    const auto* minConstant = asConstant(foldConstants(minPayoff));
    ASSERT_NE(minConstant, nullptr);
    EXPECT_DOUBLE_EQ(minConstant->getValue(), 1.0);
}

TEST(ConstantFoldVisitorTest, GreaterThanTest) {
    const auto greaterThan = constant(1.0) > constant(1.0);
    const auto* greaterThanConstant = asConstant(foldConstants(greaterThan));
    ASSERT_NE(greaterThanConstant, nullptr);
    EXPECT_DOUBLE_EQ(greaterThanConstant->getValue(), 0.0);

    const auto greaterThanOrEqual = constant(1.0) >= constant(1.0);
    const auto* greaterThanOrEqualConstant = asConstant(foldConstants(greaterThanOrEqual));
    ASSERT_NE(greaterThanOrEqualConstant, nullptr);
    EXPECT_DOUBLE_EQ(greaterThanOrEqualConstant->getValue(), 1.0);
}

TEST(ConstantFoldVisitorTest, IfThenElseTest) {
    {
        const auto ifThenElse = ite(1.0 > 0.0, 2.0, 3.0);
        const auto* ifThenElseConstant = asConstant(foldConstants(ifThenElse));
        ASSERT_NE(ifThenElseConstant, nullptr);
        EXPECT_DOUBLE_EQ(ifThenElseConstant->getValue(), 2.0);
    }

    {
        const auto ifThenElse = ite(1.0 > 2.0, 2.0, 3.0);
        const auto* ifThenElseConstant = asConstant(foldConstants(ifThenElse));
        ASSERT_NE(ifThenElseConstant, nullptr);
        EXPECT_DOUBLE_EQ(ifThenElseConstant->getValue(), 3.0);
    }
}
