#include <iomanip>
#include <iostream>

#include "numerics/linear/PiecewiseLinearFunction.h"
#include "payoff/PayoffNode.h"
#include "payoff/PiecewiseLinearFunctionVisitor.h"
#include "payoff/types.h"

using namespace payoff;
using namespace numerics::linear;

int main() {
    {
        // Example: Barrier Enhanced Notes
        constexpr double notional = 100.0;
        constexpr double barrier = 80.0;
        constexpr double cap = 120.0;

        const auto S = fixing("SPY", makeDate(2026, 3, 20));

        // Three regions:
        // S <  barrier:            S              (barrier breached, lose from initial)
        // barrier <= S < notional: notional       (protected)
        // S >= notional:           min(S, cap)    (participate up to cap)

        const auto barrierCondition = S >= constant(barrier);
        const auto protectedPayoff = min(max(S, constant(notional)), constant(cap));
        const auto breachedPayoff = S;

        const auto payoff = ite(barrierCondition, protectedPayoff, breachedPayoff);

        const auto plf = PLFVisitor().evaluate(payoff);

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

            std::cout << std::fixed << std::setprecision(1) << std::setw(7) << spot << "  |"
                      << std::setw(8) << plf(spot) << "  | " << region << "\n";
        }
    }

    return 0;
}
