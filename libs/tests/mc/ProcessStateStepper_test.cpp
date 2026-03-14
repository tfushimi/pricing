#include "mc/ProcessStateStepper.h"

#include <gtest/gtest.h>

#include "MCTestUtils.h"
#include "market/ConstantForwardCurve.h"
#include "mc/Process.h"
#include "mc/TimeGrid.h"

using namespace market;
using namespace mc;
using namespace std::chrono;

class ProcessStateStepperTest : public ::testing::Test {
   protected:
    const Date pricingDate = makeDate(2025, 1, 1);
    const ConstantForwardCurve forward{pricingDate, 100, 0.01};
    const GBMProcess gbm{[&](const double T) { return forward.get(T); }, 0.2};
    ConstantRNG constRng{0.0};

    TimeGrid makeAnnualGrid(const int years) const {
        std::vector<Date> fixings;
        for (int i = 0; i < years; i++) {
            fixings.push_back(year(2025 + i) / January / 1d);
        }
        return TimeGrid{fixings, pricingDate, 1.0 / 12.0};
    }
};

TEST_F(ProcessStateStepperTest, ScenarioHasCorrectNumberOfFixings) {
    const ProcessStateStepper stepper(gbm);

    const auto grid = makeAnnualGrid(3);

    const auto scenario = stepper.run(grid, 100, constRng);
    EXPECT_EQ(scenario.size(), 3);

    EXPECT_TRUE(scenario.count(2025y / January / 1d));
    EXPECT_TRUE(scenario.count(2026y / January / 1d));
    EXPECT_TRUE(scenario.count(2027y / January / 1d));
}

TEST_F(ProcessStateStepperTest, GBMPureDriftPath) {
    const ProcessStateStepper stepper(gbm);

    const auto grid = makeAnnualGrid(1);

    const auto scenario = stepper.run(grid, 1, constRng);

    constexpr auto simulationDate = 2025y / January / 1d;
    const Sample& spotAtT = scenario.at(simulationDate);
    EXPECT_NEAR(mean(spotAtT), forward.get(yearFraction(pricingDate, simulationDate)), 1e-4);
}

// TODO use random number