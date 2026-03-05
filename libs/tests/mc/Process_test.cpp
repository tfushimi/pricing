#include "mc/Process.h"

#include <gtest/gtest.h>

#include "MCTestUtils.h"
#include "market/ConstantForwardCurve.h"

using namespace market;
using namespace mc;

class ProcessTest : public ::testing::Test {
   protected:
    const Date pricingDate = makeDate(2025, 1, 1);
    const ConstantForwardCurve forward{pricingDate, 100, 0.01};
};

TEST_F(ProcessTest, InitialState) {
    const GBMProcess gbm(forward, 0.2);
    const auto [logS] = gbm.initialState(1000);
    EXPECT_NEAR(mean(logS), 0.0, 1e-10);
    EXPECT_EQ(gbm.nNormals(), 1);
}

TEST_F(ProcessTest, GBMZeroDiffusion) {
    constexpr double vol = 0.2;
    constexpr double dt = 1.0;

    const GBMProcess gbm(forward, vol);
    const auto state0 = gbm.initialState(1);

    const std::vector dW = {Sample(0.0, 1)};
    const auto state1 = gbm.step(state0, 0.0, dt, dW);

    EXPECT_NEAR(mean(state1.logZ), 0.0, 1e-10);

    const Sample& spot = gbm.value(state1, dt);
    EXPECT_NEAR(mean(spot), forward.get(dt), 1e-10);
}

TEST_F(ProcessTest, HestonZeroDiffusion) {
    constexpr double v0 = 0.04;  // =theta to keep v stay at theta
    constexpr double theta = v0;
    constexpr double dt = 1.0;

    const HestonProcess heston(forward, v0, 2.0, theta, 0.3, -0.7);
    const auto state0 = heston.initialState(1);

    const std::vector dW = {Sample(0.0, 1), Sample(0.0, 1)};
    const auto state1 = heston.step(state0, 0.0, dt, dW);

    EXPECT_NEAR(mean(state1.v), v0, 1e-10);

    EXPECT_NEAR(mean(state1.logZ), 0.0, 1e-10);

    const Sample& spot = heston.value(state1, dt);
    EXPECT_NEAR(mean(spot), forward.get(dt), 1e-10);
}
