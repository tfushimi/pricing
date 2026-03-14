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
    const GBMProcess gbm([&](const double T) { return forward.get(T); }, 0.2);
    const auto [logS] = gbm.initialState(1000);
    EXPECT_NEAR(mean(logS), 0.0, 1e-10);
    EXPECT_EQ(gbm.nNormals(), 1);
}

TEST_F(ProcessTest, GBMZeroDiffusion) {
    constexpr double dt = 1.0;

    const GBMProcess gbm([&](const double T) { return forward.get(T); }, 0.2);
    const auto state0 = gbm.initialState(1);

    const std::vector dW = {Sample(0.0, 1)};
    const auto state1 = gbm.step(state0, 0.0, dt, dW);

    EXPECT_NEAR(mean(state1.logZ), 0.0, 1e-10);

    const Sample& spot = gbm.value(state1, dt);
    EXPECT_NEAR(mean(spot), forward.get(dt), 1e-10);
}

TEST_F(ProcessTest, GBMDiffusion) {
    constexpr std::size_t nPaths = 100'000;
    constexpr double T = 1.0;
    constexpr double vol = 0.2;

    const GBMProcess gbm([&](const double t) { return forward.get(t); }, vol);

    constexpr int nSteps = 12;
    constexpr double dt = T / nSteps;

    RNG rng(42);
    auto state = gbm.initialState(nPaths);
    for (int i = 0; i < nSteps; ++i) {
        std::vector dW = {Sample(0.0, nPaths)};
        rng.fill(dW[0]);
        state = gbm.step(state, i * dt, dt, dW);
    }

    const Sample spot = gbm.value(state, T);
    EXPECT_NEAR(mean(spot), forward.get(T), 1e-10);

    constexpr double expectedVar = vol * vol * T;
    EXPECT_NEAR(variance(state.logZ), expectedVar, 1e-3);
}

TEST_F(ProcessTest, HestonZeroDiffusion) {
    constexpr double v0 = 0.04;  // =theta to keep v stay at theta
    constexpr double theta = v0;
    constexpr double dt = 1.0;

    const HestonProcess heston([&](const double T) { return forward.get(T); },
                               {v0, 2.0, theta, 0.3, -0.7});
    const auto state0 = heston.initialState(1);

    const std::vector dW = {Sample(0.0, 1), Sample(0.0, 1)};
    const auto state1 = heston.step(state0, 0.0, dt, dW);

    EXPECT_NEAR(mean(state1.v), v0, 1e-10);

    EXPECT_NEAR(mean(state1.logZ), 0.0, 1e-10);

    const Sample& spot = heston.value(state1, dt);
    EXPECT_NEAR(mean(spot), forward.get(dt), 1e-10);
}

TEST_F(ProcessTest, HestonDiffusion) {
    constexpr std::size_t nPaths = 100'000;
    constexpr double T = 1.0;
    constexpr double v0 = 0.04;
    constexpr double kappa = 2.0;
    constexpr double theta = v0;

    const HestonProcess heston([&](const double t) { return forward.get(t); },
                               {v0, kappa, theta, 0.3, -0.7});

    constexpr int nSteps = 12;
    constexpr double dt = T / nSteps;

    RNG rng(42);
    auto state = heston.initialState(nPaths);
    for (int i = 0; i < nSteps; ++i) {
        std::vector dW = {Sample(0.0, nPaths), Sample(0.0, nPaths)};
        rng.fill(dW[0]);
        rng.fill(dW[1]);
        state = heston.step(state, i * dt, dt, dW);
    }

    const Sample spot = heston.value(state, T);
    EXPECT_NEAR(mean(spot), forward.get(T), 1e-10);

    const double expectedVar = theta * T + (v0 - theta) * (1.0 - std::exp(-kappa * T)) / kappa;
    EXPECT_NEAR(variance(state.logZ), expectedVar, 1e-3);
}