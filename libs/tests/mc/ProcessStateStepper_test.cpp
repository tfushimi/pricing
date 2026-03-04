#include "mc/ProcessStateStepper.h"

#include <gtest/gtest.h>

#include "MCTestUtils.h"
#include "mc/Process.h"
#include "mc/TimeGrid.h"

using namespace mc;
using namespace std::chrono;

static TimeGrid makeAnnualGrid(const int years) {
    constexpr Date start = 2024y / January / 1d;
    std::vector<Date> fixings;
    for (int y = 1; y <= years; ++y) {
        fixings.push_back((year(2024 + y)) / January / 1d);
    }
    return TimeGrid{fixings, start, 1.0 / 12.0};
}

TEST(ProcessStateStepper, ScenarioHasCorrectNumberOfFixings) {
    const GBMProcess gbm(100.0, 0.2);
    const ProcessStateStepper stepper(gbm);

    const auto grid = makeAnnualGrid(3);
    ConstantRNG rng(0.0);

    const auto scenario = stepper.run(grid, 100, rng);
    EXPECT_EQ(scenario.size(), 3);

    EXPECT_TRUE(scenario.count(2025y / January / 1d));
    EXPECT_TRUE(scenario.count(2026y / January / 1d));
    EXPECT_TRUE(scenario.count(2027y / January / 1d));
}

TEST(ProcessStateStepper, GBMPureDriftPath) {
    constexpr double F0 = 100.0;
    constexpr double vol = 0.2;

    const GBMProcess gbm(F0, vol);
    const ProcessStateStepper stepper(gbm);

    const auto grid = makeAnnualGrid(1);
    ConstantRNG rng(0.0);

    const auto scenario = stepper.run(grid, 1, rng);

    const double T = grid.time(grid.size() - 1);
    const Sample& spotAtT = scenario.at(2025y / January / 1d);
    const double expected = F0 * std::exp(-0.5 * vol * vol * T);
    EXPECT_NEAR(mean(spotAtT), expected, 1e-4);
}