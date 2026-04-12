#pragma once

#include <functional>

namespace numerics::integration {

inline double integrate(const std::function<double(double)>& f) {
    constexpr double du = 0.001;
    constexpr double uMax = 200.0;
    double sum = 0.0;
    for (double u = du; u < uMax; u += du) {
        sum += f(u) * du;
    }
    return sum;
}
}