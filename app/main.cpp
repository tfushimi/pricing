#include <iostream>

#include "payoff/FixingDate.h"
#include "numerics/linear/PiecewiseLinearFunction.h"
#include "numerics/types.h"
#include "payoff/PLVisitor.h"
#include "payoff/PayoffNode.h"

using namespace payoff;
using namespace numerics::linear;

int main() {

    // PL: Call Option
    {
        const auto s = PL::linear(1.0, 0.0);
        const auto k = PL::constant(100.0);
        const auto callPayoff = PL::max(s - k, PL::constant(0.0));

        std::cout << callPayoff.toString() << std::endl;
        std::cout << " S=50:   " << callPayoff(50.0) << std::endl;  // 0
        std::cout << " S=100:  " << callPayoff(100.0) << std::endl; // 0
        std::cout << " S=150: " << callPayoff(150.0) << std::endl;  // 50
    }

    // DSL: Call Option
    {
        const auto S = fixing("2026-03-20");
        const auto callPayoff = max(S - 100.0, 0.0);

        PLVisitor plVisitor;
        const auto callPL = plVisitor.evaluate(callPayoff);

        std::cout << "Call payoff PL:\n" << callPL.toString() << "\n";
        std::cout << " S=50:   " << callPL(50.0) << std::endl;  // 0
        std::cout << " S=100:  " << callPL(100.0) << std::endl; // 0
        std::cout << " S=150: " << callPL(150.0) << std::endl;  // 50
    }

    // DSL: Digital Option
    {
        const auto S = fixing("2026-03-20");
        const auto digitalPayoff = ite(S >= 100, 1.0, 0.0);

        PLVisitor plVisitor;
        const auto digitalPL = plVisitor.evaluate(digitalPayoff);

        std::cout << "Digital payoff PL:\n" << digitalPL.toString() << "\n";
        std::cout << " S=50:   " << digitalPL(50.0) << std::endl;  // 0
        std::cout << " S=100:  " << digitalPL(100.0) << std::endl; // 1
        std::cout << " S=150: " << digitalPL(150.0) << std::endl;  // 1
    }

    // DSL: Double Digital Option
    {
        const auto S = fixing("2026-03-20");
        const auto doubleDigitalPayoff = (S > 90) - (S > 110);

        PLVisitor plVisitor;
        const auto doubleDigitalPL = plVisitor.evaluate(doubleDigitalPayoff);

        std::cout << "Double digital payoff PL:\n" << doubleDigitalPL.toString() << "\n";
        std::cout << " S=50:   " << doubleDigitalPL(50.0) << std::endl;  // 0
        std::cout << " S=100:  " << doubleDigitalPL(100.0) << std::endl; // 1
        std::cout << " S=150: " << doubleDigitalPL(150.0) << std::endl;  // 0
    }

    return 0;
}
