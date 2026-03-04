#include "mc/Process.h"

#include <gtest/gtest.h>

#include "MCTestUtils.h"

TEST(GBMProcess, InitialState) {
    const mc::GBMProcess gbm(0.2);
    const auto [logS] = gbm.initialState(1000);
    EXPECT_NEAR(mean(logS), 0.0, 1e-10);
    EXPECT_EQ(gbm.nNormals(), 1);
}

TEST(GBMProcess, ZeroDiffusion) {
    constexpr double vol = 0.2;
    constexpr double dt = 1.0;

    const mc::GBMProcess gbm(vol);
    const auto state0 = gbm.initialState(1);

    const std::vector dW = {Sample(0.0, 1)};
    const auto state1 = gbm.step(state0, 0.0, dt, dW);

    EXPECT_NEAR(mean(state1.logZ), 0.0, 1e-10);

    const Sample& spot = gbm.value(state1);
    EXPECT_NEAR(mean(spot), 1.0, 1e-10);
}

TEST(HestonProcess, ZeroDiffusion) {
    constexpr double v0 = 0.04;  // =theta to keep v stay at theta
    constexpr double theta = v0;
    constexpr double dt = 1.0;

    const mc::HestonProcess heston(v0, 2.0, theta, 0.3, -0.7);
    const auto state0 = heston.initialState(1);

    const std::vector dW = {Sample(0.0, 1), Sample(0.0, 1)};
    const auto state1 = heston.step(state0, 0.0, dt, dW);

    EXPECT_NEAR(mean(state1.v), v0, 1e-10);

    EXPECT_NEAR(mean(state1.logZ), 0.0, 1e-10);

    const Sample& spot = heston.value(state1);
    EXPECT_NEAR(mean(spot), 1.0, 1e-10);
}
