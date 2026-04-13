#pragma once

#include "common/Types.h"
#include "numerics/RootFinding.h"
#include "pricer/BSFormula.h"
#include "pricer/HestonFormula.h"

namespace pricer {

/**
 * Compute implied volatility by inverting the Black formula via bisection.
 *
 * @param price = call price to invert
 * @param F     = forward price
 * @param K     = strike
 * @param T     = time to maturity (years)
 * @param dF    = discount factor
 * @param tol   = convergence tolerance on price
 * @return implied volatility
 */
inline double impliedVol(const double price, const double F, const double K, const double T,
                         const double dF, const double tol = 1e-8) {
    using namespace numerics::rootfinding;
    return bisection<double>(price, 1e-6, 10.0, tol,
                             [&](const double sigma) { return bsCallFormula(F, K, T, dF, sigma); });
}

/**
 * Compute Heston implied volatility: price a European call via the Heston
 * characteristic function, then invert the Black formula.
 *
 * @param F      = forward price
 * @param K      = strike
 * @param T      = time to maturity (years)
 * @param dF     = discount factor
 * @param params = Heston parameters
 * @return implied volatility
 */
inline double hestonImpliedVol(const double F, const double K, const double T, const double dF,
                               const HestonParams& params) {
    const double price = hestonCallFormula(F, K, T, dF, params);
    return impliedVol(price, F, K, T, dF);
}

}  // namespace pricer
