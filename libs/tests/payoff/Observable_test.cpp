#include "payoff/Observable.h"

#include <gtest/gtest.h>

#include "payoff/Transforms.h"

using namespace payoff;

const auto S = fixing("SPY", makeDate(2026, 3, 20));

TEST(ObservableTest, ArithmeticTest) {
    const auto add = constant(1.0) + constant(2.0);
    const auto addPLF = toPiecewiseLinearFunction(add);
    EXPECT_DOUBLE_EQ(addPLF(-1e+10), 3.0);
    EXPECT_DOUBLE_EQ(addPLF(0.0), 3.0);
    EXPECT_DOUBLE_EQ(addPLF(1e+10), 3.0);

    const auto sumNode = sum(1.0, 2.0, 3.0);
    const auto sumPLF = toPiecewiseLinearFunction(sumNode);
    EXPECT_DOUBLE_EQ(sumPLF(-1e+10), 6.0);
    EXPECT_DOUBLE_EQ(sumPLF(0.0), 6.0);
    EXPECT_DOUBLE_EQ(sumPLF(1e+10), 6.0);

    const auto sub = constant(1.0) - constant(2.0);
    const auto subPLF = toPiecewiseLinearFunction(sub);
    EXPECT_DOUBLE_EQ(subPLF(-1e+10), -1.0);
    EXPECT_DOUBLE_EQ(subPLF(0.0), -1.0);
    EXPECT_DOUBLE_EQ(subPLF(1e+10), -1.0);

    const auto mult = constant(2.0) * constant(3.0);
    const auto multPLF = toPiecewiseLinearFunction(mult);
    EXPECT_DOUBLE_EQ(multPLF(-1e+10), 6.0);
    EXPECT_DOUBLE_EQ(multPLF(0.0), 6.0);
    EXPECT_DOUBLE_EQ(multPLF(1e+10), 6.0);

    const auto divide = constant(1.0) / constant(2.0);
    const auto dividePLF = toPiecewiseLinearFunction(divide);
    EXPECT_DOUBLE_EQ(dividePLF(-1e+10), 0.5);
    EXPECT_DOUBLE_EQ(dividePLF(0.0), 0.5);
    EXPECT_DOUBLE_EQ(dividePLF(1e+10), 0.5);

    const auto s1 = constant(50.0);
    const auto s2 = constant(100.0);
    const auto s3 = constant(150.0);
    const auto maxPLF = toPiecewiseLinearFunction(max(s1, s2, s3));
    EXPECT_DOUBLE_EQ(maxPLF(-1e+10), 150.0);
    EXPECT_DOUBLE_EQ(maxPLF(0.0), 150.0);
    EXPECT_DOUBLE_EQ(maxPLF(1e+10), 150.0);

    const auto minPLF = toPiecewiseLinearFunction(min(s1, s2, s3));
    EXPECT_DOUBLE_EQ(minPLF(-1e+10), 50.0);
    EXPECT_DOUBLE_EQ(minPLF(0), 50.0);
    EXPECT_DOUBLE_EQ(minPLF(1e+10), 50.0);
}

TEST(ObservableTest, FixingArithmeticTest) {
    const auto add = S + 50;
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(add)(30.0), 80.0);

    const auto sub = S - 50;
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(sub)(80.0), 30.0);

    const auto neg = -S;
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(neg)(5.0), -5.0);

    const auto mult = S * 3.0;
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(mult)(4.0), 12.0);

    const auto divide = S / 2.0;
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(divide)(6.0), 3.0);

    const auto sum1 = sum(S, 50);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(sum1)(30.0), 80.0);

    const auto sum2 = sum(S, 50, 100);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(sum2)(30.0), 180.0);

    const auto sum3 = sum(S, S, S);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(sum3)(30.0), 90.0);
}

TEST(ObservableTest, GreaterThanTest) {
    const auto greaterThanK = S > 50.0;
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(greaterThanK)(40.0), 0.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(greaterThanK)(50.0), 0.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(greaterThanK)(60.0), 1.0);

    const auto greaterThanOrEqualToK = S >= 50.0;
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(greaterThanOrEqualToK)(40.0), 0.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(greaterThanOrEqualToK)(50.0), 1.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(greaterThanOrEqualToK)(60.0), 1.0);

    const auto one = constant(3.0) > constant(2.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(one)(-1e10), 1.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(one)(0), 1.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(one)(1e10), 1.0);
}

TEST(ObservableTest, IfThenElseTest) {
    const auto K = constant(100.0);

    const auto cond = S > K;
    const auto then_ = S - K;
    const auto call = ite(cond, then_, constant(0.0));

    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(call)(50.0), 0.0) << "call: below strike";
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(call)(150.0), 50.0) << "call: above strike";
}

TEST(ObservableTest, MaxMinTest) {
    // call
    const auto call = max(S - 100.0, 0.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(call)(50.0), 0.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(call)(150.0), 50.0);

    // call spread
    const auto capped = min(200, max(S - 100, 10));
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(capped)(50.0), 10.0)
        << "floored+capped: floor region";
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(capped)(150.0), 50.0)
        << "floored+capped: linear region";
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(capped)(400.0), 200.0)
        << "floored+capped: cap region";

    // best-of: max(S-90, S-100, S-110, 0) = S-90 for S>90
    const auto bestOf = max(S - 90.0, S - 100.0, S - 110.0, 0.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(bestOf)(150.0), 60.0);
    EXPECT_DOUBLE_EQ(toPiecewiseLinearFunction(bestOf)(50.0), 0.0);
}