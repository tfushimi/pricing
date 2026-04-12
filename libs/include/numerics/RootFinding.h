#pragma once

#include <cmath>
#include <functional>

namespace numerics::rootfinding {

template <typename T>
double bisection(double target, double lower, double upper, double tol, const std::function<double(double)>& f) {
    double sol = 0.5 * (lower + upper);
    double y = f(sol);

    while ((std::fabs(y - target) > tol)) {
        if (y < target) {
            lower = sol;
        } else {
            upper = sol;
        }
        sol = 0.5 * (lower + upper);
        y = f(sol);
    };
    return sol;
}
}  // namespace numerics::rootfinding
