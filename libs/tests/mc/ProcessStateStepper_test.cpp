#include "mc/ProcessStateStepper.h"

#include <gtest/gtest.h>

#include "MCTestUtils.h"
#include "common/Date.h"
#include "market/ConstantForwardCurve.h"
#include "mc/Process.h"
#include "mc/TimeGrid.h"

using namespace calendar;
using namespace market;
using namespace mc;
using namespace rng;
using namespace std::chrono;

class ProcessStateStepperTest : public ::testing::Test {
   protected:
    const Date pricingDate = makeDate(2025, 1, 1);
    const ConstantForwardCurve forward{pricingDate, 100, 0.01};
    const double vol = 0.2;
    const GBMProcess gbm{forward, vol};
};

TEST_F(ProcessStateStepperTest, ScenarioHasCorrectNumberOfFixings) {
    const ProcessStateStepper stepper(gbm);

    const auto fixingDates = {
        makeDate(2026, 1, 1),
        makeDate(2027, 1, 1),
    };
    const auto grid = TimeGrid{fixingDates, pricingDate, 1.0 / 12.0};

    const auto scenario = stepper.run(grid, 100, std::make_unique<ConstantRNG>(1.0));
    EXPECT_EQ(scenario.size(), 2);

    EXPECT_TRUE(scenario.count(2026y / January / 1d));
    EXPECT_TRUE(scenario.count(2027y / January / 1d));
}

TEST_F(ProcessStateStepperTest, GBMPureDriftPath) {
    const ProcessStateStepper stepper(gbm);

    const auto simulationDate = makeDate(2206, 1, 1);
    const auto fixingDates = {simulationDate};
    const auto grid = TimeGrid{fixingDates, pricingDate, 1.0 / 12.0};

    const auto scenario = stepper.run(grid, 1, std::make_unique<ConstantRNG>(0.0));

    const Sample& spotAtT = scenario.at(simulationDate);
    const auto T = yearFraction(pricingDate, simulationDate);
    EXPECT_NEAR(mean(spotAtT), forward(T) * std::exp(-0.5 * vol * vol * T), 1e-4);
}
