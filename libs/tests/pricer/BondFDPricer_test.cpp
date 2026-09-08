#include <vector>
#include "pricer/BondFDPricer.h"

#include <gtest/gtest.h>

constexpr double r0 = 0.01;
const std::vector taus{1.0, 5.0, 10.0, 30.0};

using namespace pricer::bond;

TEST(BondFDPricerTest, Vasicek) {

    const Affine affine = vasicekCoefficients(0.5, 0.03, 0.02);

    for (const double tau : taus) {
        const double price = bondFDPricer(affine, r0, tau, -0.2, 0.26, 400, 400);
        const double expected = vasicekFormula(0.5, 0.03, 0.02, tau, r0);
        EXPECT_NEAR(price, expected, 1e-6) << "tau = " << tau;
    }
}

TEST(BondFDPricerTest, CIR) {

    const Affine affine = cirCoefficients(0.5, 0.03, 0.08);

    for (const double tau : taus) {
        const double price = bondFDPricer(affine, r0, tau, 0.0, 0.4, 400, 400);
        const double expected = cirFormula(0.5, 0.03, 0.08, tau, r0);
        EXPECT_NEAR(price, expected, 1e-6) << "tau = " << tau;
    }
}

// Central differences in r and Crank-Nicolson in tau are both second order, so the error
// is O(h^2): halving h must divide it by four, and log2 of the ratio recovers the exponent.
TEST(BondFDPricerTest, SecondOrderConvergence) {

    const Affine affine = cirCoefficients(0.5, 0.03, 0.08);
    const double exact = cirFormula(0.5, 0.03, 0.08, 10.0, r0);

    for (const int m : {200, 400, 800}) {
        const double coarse = std::abs(bondFDPricer(affine, r0, 10.0, 0.0, 0.4, m, m) - exact);
        const double fine = std::abs(bondFDPricer(affine, r0, 10.0, 0.0, 0.4, 2 * m, 2 * m) - exact);
        const double order = std::log2(coarse / fine);
        EXPECT_NEAR(order, 2.0, 0.1) << "m = " << m << " -> " << 2 * m;
    }
}
