#include <gtest/gtest.h>

#include "ObservableTestUtils.h"
#include "payoff/Observable.h"
#include "payoff/Transforms.h"

using namespace payoff;

const auto spy = fixing("SPY", makeDate(2026, 3, 20));

TEST(ConstantFoldTest, ArithmeticTest) {
    const auto add = constant(1.0) + constant(2.0);
    const auto* addConstant = asNode<Constant>(foldConstants(add));
    ASSERT_NE(addConstant, nullptr);
    EXPECT_DOUBLE_EQ(addConstant->getValue(), 3.0);

    const auto sub = constant(1.0) - constant(2.0);
    const auto* subConstant = asNode<Constant>(foldConstants(sub));
    ASSERT_NE(subConstant, nullptr);
    EXPECT_DOUBLE_EQ(subConstant->getValue(), -1.0);

    const auto mult = constant(2.0) * constant(3.0);
    const auto* multConstant = asNode<Constant>(foldConstants(mult));
    ASSERT_NE(multConstant, nullptr);
    EXPECT_DOUBLE_EQ(multConstant->getValue(), 6.0);

    const auto divide = constant(1.0) / constant(2.0);
    const auto* divideConstant = asNode<Constant>(foldConstants(divide));
    ASSERT_NE(divideConstant, nullptr);
    EXPECT_DOUBLE_EQ(divideConstant->getValue(), 0.5);

    const auto sum1 = sum(1.0, 2.0, 3.0);
    const auto* sum1Constant = asNode<Constant>(foldConstants(sum1));
    ASSERT_NE(sum1Constant, nullptr);
    EXPECT_DOUBLE_EQ(sum1Constant->getValue(), 6.0);
}

TEST(ConstantFoldTest, SumTest) {
    {
        const auto sumNode = sum(spy);
        const auto foldedNode = foldConstants(sumNode);
        const auto* fixing = asNode<Fixing>(foldedNode);
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }

    {
        const auto sumNode = sum(spy, 0.0);
        const auto foldedNode = foldConstants(sumNode);
        const auto* fixing = asNode<Fixing>(foldedNode);
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }

    {
        const auto sumNode = sum(spy, 1.0, 1.0);
        const auto foldedNode = foldConstants(sumNode);
        const auto* sumPtr = asNode<Sum>(foldedNode);
        ASSERT_NE(sumPtr, nullptr);
        ASSERT_EQ(sumPtr->size(), 2);

        const auto* fixing = asNode<Fixing>(*sumPtr->begin());
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");

        const auto two = asNode<Constant>(*std::next(sumPtr->begin()));
        ASSERT_NE(two, nullptr);
        ASSERT_EQ(two->getValue(), 2.0);
    }
}

TEST(ConstantFoldTest, MinTest) {
    {
        const auto minNode = min(spy);
        const auto foldedNode = foldConstants(minNode);
        const auto* fixing = asNode<Fixing>(foldedNode);
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }

    {
        const auto minNode = min(spy, 1.0, 0.0);
        const auto foldedNode = foldConstants(minNode);
        const auto* minPtr = asNode<Min>(foldedNode);
        ASSERT_NE(minPtr, nullptr);
        ASSERT_EQ(minPtr->size(), 2);

        const auto* fixing = asNode<Fixing>(*minPtr->begin());
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");

        const auto zero = asNode<Constant>(*std::next(minPtr->begin()));
        ASSERT_NE(zero, nullptr);
        ASSERT_EQ(zero->getValue(), 0.0);
    }
}

TEST(ConstantFoldTest, MaxTest) {
    {
        const auto maxNode = max(spy);
        const auto foldedNode = foldConstants(maxNode);
        const auto* fixing = asNode<Fixing>(foldedNode);
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }

    {
        const auto maxNode = max(spy, 1.0, 0.0);
        const auto foldedNode = foldConstants(maxNode);
        const auto* minPtr = asNode<Max>(foldedNode);
        ASSERT_NE(minPtr, nullptr);
        ASSERT_EQ(minPtr->size(), 2);

        const auto* fixing = asNode<Fixing>(*minPtr->begin());
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");

        const auto one = asNode<Constant>(*std::next(minPtr->begin()));
        ASSERT_NE(one, nullptr);
        ASSERT_EQ(one->getValue(), 1.0);
    }
}

TEST(ConstantFoldTest, MultiplyTest) {
    const auto zero = constant(0.0);
    const auto one = constant(1.0);

    {
        const auto mult = spy * zero;
        const auto* multConstant = asNode<Constant>(foldConstants(mult));
        ASSERT_NE(multConstant, nullptr);
        EXPECT_DOUBLE_EQ(multConstant->getValue(), 0.0);
    }

    {
        const auto mult = zero * spy;
        const auto* multConstant = asNode<Constant>(foldConstants(mult));
        ASSERT_NE(multConstant, nullptr);
        EXPECT_DOUBLE_EQ(multConstant->getValue(), 0.0);
    }

    {
        const auto mult = spy * one;
        const auto* fixing = asNode<Fixing>(foldConstants(mult));
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }

    {
        const auto mult = one * spy;
        const auto* fixing = asNode<Fixing>(foldConstants(mult));
        ASSERT_NE(fixing, nullptr);
        EXPECT_EQ(fixing->getSymbol(), "SPY");
    }
}

TEST(ConstantFoldTest, DivideTest) {
    const auto divide = spy / constant(1.0);
    const auto* divideConstant = asNode<Fixing>(foldConstants(divide));
    ASSERT_NE(divideConstant, nullptr);
    EXPECT_EQ(divideConstant->getSymbol(), "SPY");
}

TEST(ConstantFoldTest, DivideByZeroThrows) {
    const auto divide = constant(1.0) / constant(0.0);
    EXPECT_THROW(foldConstants(divide), std::invalid_argument);
}

TEST(ConstantFoldTest, MaxMinTest) {
    const auto maxPayoff = max(constant(1.0), constant(2.0));
    const auto* maxConstant = asNode<Constant>(foldConstants(maxPayoff));
    ASSERT_NE(maxConstant, nullptr);
    EXPECT_DOUBLE_EQ(maxConstant->getValue(), 2.0);

    const auto minPayoff = min(constant(1.0), constant(2.0));
    const auto* minConstant = asNode<Constant>(foldConstants(minPayoff));
    ASSERT_NE(minConstant, nullptr);
    EXPECT_DOUBLE_EQ(minConstant->getValue(), 1.0);
}

TEST(ConstantFoldTest, GreaterThanTest) {
    const auto greaterThan = constant(1.0) > constant(1.0);
    const auto* greaterThanConstant = asNode<Constant>(foldConstants(greaterThan));
    ASSERT_NE(greaterThanConstant, nullptr);
    EXPECT_DOUBLE_EQ(greaterThanConstant->getValue(), 0.0);

    const auto greaterThanOrEqual = constant(1.0) >= constant(1.0);
    const auto* greaterThanOrEqualConstant = asNode<Constant>(foldConstants(greaterThanOrEqual));
    ASSERT_NE(greaterThanOrEqualConstant, nullptr);
    EXPECT_DOUBLE_EQ(greaterThanOrEqualConstant->getValue(), 1.0);
}

TEST(ConstantFoldTest, IfThenElseTest) {
    {
        const auto ifThenElse = ite(1.0 > 0.0, 2.0, 3.0);
        const auto* ifThenElseConstant = asNode<Constant>(foldConstants(ifThenElse));
        ASSERT_NE(ifThenElseConstant, nullptr);
        EXPECT_DOUBLE_EQ(ifThenElseConstant->getValue(), 2.0);
    }

    {
        const auto ifThenElse = ite(1.0 > 2.0, 2.0, 3.0);
        const auto* ifThenElseConstant = asNode<Constant>(foldConstants(ifThenElse));
        ASSERT_NE(ifThenElseConstant, nullptr);
        EXPECT_DOUBLE_EQ(ifThenElseConstant->getValue(), 3.0);
    }
}
