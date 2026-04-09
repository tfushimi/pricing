#include "mc/Process.h"

#include <gtest/gtest.h>

#include "MCTestUtils.h"
#include "common/Date.h"
#include "market/ConstantForwardCurve.h"

using namespace calendar;
using namespace market;
using namespace mc;

class ProcessTest : public ::testing::Test {
   protected:
    const Date pricingDate = makeDate(2025, 1, 1);
    const ConstantForwardCurve forward{pricingDate, 100, 0.01};
};

TEST_F(ProcessTest, GBMInitialState) {
    const GBMProcess gbm(forward, 0.2);
    const auto [logS] = gbm.initialState(1000);
    EXPECT_EQ(logS.size(), 1000);
    EXPECT_NEAR(mean(logS), 0.0, 1e-10);
    EXPECT_EQ(gbm.nNormals(), 1);
}

TEST_F(ProcessTest, GBMStepper) {
    constexpr double vol = 0.2;
    constexpr double dt = 1.0;

    const GBMProcess gbm(forward, vol);

    auto state = gbm.initialState(1);

    // Step 1
    // logZ1 = (-0.5 * 0.04) * 1.0 + 0.2 * 0.1 = -0.02 + 0.02 = 0.0
    const std::vector dW1 = {Sample{0.1}};
    state = gbm.step(state, 0.0, dt, dW1);

    constexpr double logZ1 = 0.0;
    EXPECT_NEAR(state.logZ[0], logZ1, 1e-15);

    const Sample& spot1 = gbm.value(state, dt);
    EXPECT_NEAR(mean(spot1), forward(dt) * std::exp(logZ1), 1e-15);

    // Step 2
    // logZ2 = 0.0 + (-0.5 * 0.04) * 1.0 + 0.2 * 0.15 = -0.02 + 0.03 = 0.01
    const std::vector dW2 = {Sample{0.15}};
    state = gbm.step(state, dt, dt, dW2);

    constexpr double logZ2 = logZ1 + (-0.5 * vol * vol) * dt + vol * 0.15;  // 0.01
    EXPECT_NEAR(state.logZ[0], logZ2, 1e-15);

    const Sample& spot2 = gbm.value(state, dt);
    EXPECT_NEAR(mean(spot2), forward(dt) * std::exp(logZ2), 1e-15);
}

TEST_F(ProcessTest, HestonInitialState) {
    const HestonProcess heston(forward, {0.04, 2.0, 0.04, 0.3, -0.8});
    const auto [logS, v] = heston.initialState(1000);
    EXPECT_EQ(logS.size(), 1000);
    EXPECT_EQ(v.size(), 1000);
    EXPECT_NEAR(mean(logS), 0.0, 1e-10);
    EXPECT_NEAR(mean(v), 0.04, 1e-10);
    EXPECT_EQ(heston.nNormals(), 2);
}

TEST_F(ProcessTest, HestonStepper) {
    constexpr double v0 = 0.04;
    constexpr double kappa = 2.0;
    constexpr double theta = 0.05;  // != v0 so drift is nonzero
    constexpr double xi = 0.3;
    constexpr double rho = 0.0;
    constexpr double dt = 1.0;

    const HestonProcess heston(forward, {v0, kappa, theta, xi, rho});

    auto state = heston.initialState(1);

    // Step 1
    const std::vector dW1 = {Sample{0.1}, Sample{0.2}};
    state = heston.step(state, 0.0, dt, dW1);

    // v1 = v0 + kappa*(theta - v0)*dt + xi*sqrt(v0)*dW_v + 0.25*xi²*(dW_v² - dt)
    //    = 0.04 + 2.0*(0.05 - 0.04)*1.0 + 0.3*0.2*0.2 + 0.25*0.09*(0.04 - 1.0)
    //    = 0.04 + 0.02               + 0.012          - 0.0216
    //    = 0.0504
    constexpr double v1 = 0.0504;

    // logZ1 = (-0.5*v0)*dt + sqrt(v0)*dW_S
    //       = (-0.5*0.04)*1.0 + 0.2*0.1
    //       = -0.02 + 0.02 = 0.0  (Itô correction exactly cancels diffusion)
    constexpr double logZ1 = 0.0;

    EXPECT_NEAR(state.v[0], v1, 1e-15);
    EXPECT_NEAR(state.logZ[0], logZ1, 1e-15);

    const Sample& spot1 = heston.value(state, dt);
    EXPECT_NEAR(mean(spot1), forward(dt) * std::exp(logZ1), 1e-15);

    // Step 2
    const std::vector dW2 = {Sample{0.15}, Sample{0.05}};
    state = heston.step(state, dt, dt, dW2);

    const double v2 = v1 + kappa * (theta - v1) * dt + xi * std::sqrt(v1) * 0.05 +
                      0.25 * xi * xi * (0.05 * 0.05 - dt);

    const double logZ2 = logZ1 + -0.5 * v1 * dt + std::sqrt(v1) * 0.15;

    EXPECT_NEAR(state.v[0], v2, 1e-15);
    EXPECT_NEAR(state.logZ[0], logZ2, 1e-15);

    const Sample& spot2 = heston.value(state, dt);
    EXPECT_NEAR(mean(spot2), forward(dt) * std::exp(logZ2), 1e-15);
}