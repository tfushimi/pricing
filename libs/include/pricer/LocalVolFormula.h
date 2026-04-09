#pragma once

#include "common/Types.h"

namespace pricer {

template <typename T>
T approximateLocalVol(const HestonParams& params, const T& logZ, const double currentTime) {
    const auto [v0, kappa, theta, xi, rho] = params;
    const auto kappa_bar = kappa + 0.5 * xi;
    const auto theta_bar = theta * kappa / kappa_bar;
    const auto kappaT = kappa_bar * currentTime;
    const auto result = (v0 - theta_bar) * std::exp(-kappaT) + theta_bar -
                        xi * logZ * (1 - std::exp(-kappaT)) / kappaT;

    return sqrt((result + abs(result)) * 0.5);
}
}  // namespace pricer