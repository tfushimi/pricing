#include <iomanip>
#include <iostream>

#include "payoff/PayoffNode.h"
#include "payoff/Transforms.h"

using namespace payoff;
using namespace numerics::linear;

int main() {
    // Example: Barrier Enhanced Notes
    constexpr double notional = 100.0;
    constexpr double barrier = 80.0;
    constexpr double cap = 120.0;

    const auto S = fixing("SPY", makeDate(2026, 3, 20));

    // Three regions:
    // S <  barrier:            S                        (breached)
    // barrier <= S < notional: notional                 (protected)
    // S >= notional:           min(notional + 1.1*(S - notional), cap)  (1.1x participation above
    // notional)

    const auto barrierCondition = S >= constant(barrier);
    const auto participation = constant(notional) + constant(1.1) * (S - constant(notional));
    const auto protectedPayoff = min(max(participation, constant(notional)), constant(cap));
    const auto breachedPayoff = S;

    const auto payoff = ite(barrierCondition, protectedPayoff, breachedPayoff);

    const auto plf = toPiecewiseLinearFunction(payoff);

    const std::vector<double> spots = {50,  60,  70,  79,  80,  85,  90, 95,
                                       100, 105, 110, 115, 120, 130, 150};

    std::cout << "-----------------------------\n";
    std::cout << "  Spot  |  Payoff  | Region  \n";
    std::cout << "-----------------------------\n";

    for (const double spot : spots) {
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

        std::cout << std::fixed << std::setprecision(2) << std::setw(7) << spot << "  |"
                  << std::setw(8) << plf(spot) << "  | " << region << "\n";
    }

    return 0;
}
