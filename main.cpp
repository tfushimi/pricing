#include <iostream>

#include "numerics/PiecewiseLinearFunction.h"
#include "payoff/FixingDate.h"
#include "payoff/PayoffNode.h"
#include "payoff/PLVisitor.h"

using namespace payoff;
using namespace numerics::linear;

int main() {

    // PL
    {
        const auto s = PL::createLinear(1.0, 0.0);
        const auto k = PL::createConstant(100.0);
        const auto callPayoff = PL::max(s - k, PL::createConstant(0.0));

        std::cout << callPayoff.toString() << std::endl;
        std::cout << " S=50:   " << callPayoff(50.0) << std::endl; // 0
        std::cout << " S=100:  " << callPayoff(100.0) << std::endl; // 0
        std::cout << " S=150: " << callPayoff(150.0) << std::endl; // 50
    }

    // DSL
    {
        const auto S = fixing("2026-03-20");
        const auto call = max(S - 100.0, 0.0);

        PLVisitor v1;
        const auto plCall = v1.evaluate(call);

        std::cout << "Call payoff PL:\n" << plCall.toString() << "\n";
        std::cout << " S=50:   " << plCall(50.0) << std::endl; // 0
        std::cout << " S=100:  " << plCall(100.0) << std::endl; // 0
        std::cout << " S=150: " << plCall(150.0) << std::endl; // 50
    }

    return 0;
}
