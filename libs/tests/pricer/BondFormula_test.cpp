#include <vector>
#include "pricer/BondFormula.h"

#include <gtest/gtest.h>

constexpr double r0 = 0.01;
const std::vector taus{1.0, 5.0, 10.0, 30.0};

using namespace pricer::bond;

TEST(BondFormulaTest, Vasicek) {

    const std::vector expected{0.98588592, 0.89455748, 0.77518932, 0.43240181};

    for (size_t i = 0; i < taus.size(); ++i) {
        const double price = vasicekFormula(0.5, 0.03, 0.02, taus[i], r0);
        EXPECT_NEAR(price, expected[i], 1e-6);
    }
}

TEST(BondFormulaTest, CIR) {

    const std::vector expected{0.98584911, 0.89342235, 0.77252954, 0.42725815};

    for (size_t i = 0; i < taus.size(); ++i) {
        const double price = cirFormula(0.5, 0.03, 0.08, taus[i], r0);
        EXPECT_NEAR(price, expected[i], 1e-6);
    }
}