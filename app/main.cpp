#include <iomanip>
#include <iostream>

#include "market/SVI.h"
#include "market/SimpleMarket.h"
#include "payoff/Payoff.h"
#include "payoff/PayoffNode.h"
#include "payoff/Transforms.h"
#include "pricer/BSPricer.h"

using namespace market;
using namespace payoff;
using namespace pricer;
using namespace numerics::linear;
using namespace vol;

int main() {

    // 1-year tenor: meaningful time value across all regions
    const Date pricingDate = makeDate(2025, 1, 15);
    const Date fixingDate = makeDate(2026, 1, 15);
    const Date settlementDate = makeDate(2026, 1, 17);
    const double rate = 0.05;

    // Barrier Enhanced Note parameters
    constexpr double notional = 100.0;
    constexpr double barrier = 80.0;   // 20% downside protection
    constexpr double cap = 120.0;  // 20% upside cap

    const auto S = fixing("SPY", fixingDate);

    // Payoff regions:
    //   S <  80:          S                       (breached: full downside)
    //   80 <= S < 100:    100                     (protected: capital guarantee)
    //   100 <= S < 120:   100 + 1.1 * (S - 100)   (participating: 110% upside)
    //   S >= 120:         120                     (capped)
    const auto barrierCondition = S >= constant(barrier);
    const auto participation = constant(notional) + constant(1.1) * (S - constant(notional));
    const auto protectedPayoff = min(max(participation, constant(notional)), constant(cap));
    const auto breachedPayoff = S;

    const auto payoff  = ite(barrierCondition, protectedPayoff, breachedPayoff);
    const auto plf     = toPiecewiseLinearFunction(payoff);
    const auto payment = CashPayment(payoff, settlementDate);

    const std::vector<double> spots = {50,  60,  70,  79,  80,  85,  90,  95,
                                       100, 105, 110, 115, 120, 130, 150};

    // SVI surface calibrated to ~20% ATM vol with realistic equity skew:
    //   a     = 0.04   — sets the overall variance level (ATM total var ≈ 0.05 → ~22% vol)
    //   b     = 0.10   — controls the wings / vol-of-vol
    //   rho   = -0.30  — negative skew (downside puts more expensive than calls)
    //   m     = 0.00   — ATM forward at current spot
    //   sigma = 0.10   — smoothness of the smile minimum
    constexpr SVIParams sviParams{.a = 0.04, .b = 0.10, .rho = -0.30, .m = 0.00, .sigma = 0.10};

    std::cout << "--------------------------------------\n";
    std::cout << "  Spot  |  Payoff  |  Price  | Region  \n";
    std::cout << "--------------------------------------\n";

    for (const double spot : spots) {

        SimpleMarket market{pricingDate, "SPY", spot, rate, sviParams};

        std::string region;
        if (spot < barrier) {
            region = "breached";
        } else if (spot < notional) {
            region = "protected";
        } else if (spot < cap) {
            region = "participating";
        } else {
            region = "capped";
        }

        std::cout << std::fixed << std::setprecision(2)
                  << std::setw(7) << spot    << "  |"
                  << std::setw(8) << plf(spot) << "  | "
                  << std::setw(6) << bsPrice(payment, market)   << "  | " << region << "\n";
    }

    return 0;
}