#pragma once

#include <cmath>
#include <functional>

namespace numerics::rootfinding {

inline double bisection(const double target, double lower, double upper, const double tol,
                        const std::function<double(double)>& f, const int maxIter = 100) {
    double sol = 0.5 * (lower + upper);
    double y = f(sol);

    for (int i = 0; i < maxIter && std::fabs(y - target) > tol; ++i) {
        if (y < target) {
            lower = sol;
        } else {
            upper = sol;
        }
        sol = 0.5 * (lower + upper);
        y = f(sol);
    }
    return sol;
}
}  // namespace numerics::rootfinding
