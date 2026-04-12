#pragma once

#include <complex>

#include "common/Types.h"
#include "numerics/Integration.h"

namespace pricer {
/**
 * @param x = log-moneyness, log(F/K)
 * @param u = integration parameter
 * @param T = time to maturity
 * @param params = Heston parameters
 */
inline std::complex<double> hestonP0(const double u, const double x, const double T,
                                     const HestonParams& params) {
    const auto [v0, kappa, theta, xi, rho] = params;

    using Complex = std::complex<double>;
    const Complex iu{0.0, u};

    const Complex alpha = -0.5 * u * u - 0.5 * iu;
    const Complex beta = kappa - rho * xi * iu;
    const Complex d = std::sqrt(beta * beta - 2.0 * alpha * xi * xi);
    const Complex rMinus = (beta - d) / (xi * xi);
    const Complex rPlus = (beta + d) / (xi * xi);
    const Complex g = rMinus / rPlus;
    const Complex expDt = std::exp(-d * T);

    const Complex C =
        kappa * (rMinus * T - 2.0 / (xi * xi) * std::log((1.0 - g * expDt) / (1.0 - g)));
    const Complex D = rMinus * (1.0 - expDt) / (1.0 - g * expDt);

    return std::exp(C * theta + D * v0 + iu * x) / iu;
}

/**
 * @param x = log-moneyness, log(F/K)
 * @param u = integration parameter
 * @param T = time to maturity
 * @param params = Heston parameters
 */
inline std::complex<double> hestonP1(const double u, const double x, const double T,
                                     const HestonParams& params) {
    const auto [v0, kappa, theta, xi, rho] = params;

    using Complex = std::complex<double>;
    const Complex iu{0.0, u};

    const Complex alpha = -0.5 * u * u + 0.5 * iu;
    const Complex beta = kappa - rho * xi * iu - rho * xi;
    const Complex d = std::sqrt(beta * beta - 2.0 * alpha * xi * xi);
    const Complex rMinus = (beta - d) / (xi * xi);
    const Complex rPlus = (beta + d) / (xi * xi);
    const Complex g = rMinus / rPlus;
    const Complex expDt = std::exp(-d * T);

    const Complex C =
        kappa * (rMinus * T - 2.0 / (xi * xi) * std::log((1.0 - g * expDt) / (1.0 - g)));
    const Complex D = rMinus * (1.0 - expDt) / (1.0 - g * expDt);

    return std::exp(C * theta + D * v0 + iu * x) / iu;
}

/**
 * Heston call formula (based on Gatheral's formulation)
 *
 * @param F = forward price
 * @param K = strike price
 * @param T = time to maturity
 * @param dF = discount factor
 * @param params = Heston parameters
 * @return call price
 */
inline double hestonCallFormula(const double F, const double K, const double T, const double dF,
                                const HestonParams& params) {
    using namespace numerics::integration;
    const double x = std::log(F / K);  // log-moneyness, rate already absorbed into F

    const double P0 = 0.5 + (1.0 / M_PI) * integrate([&](const double u) {
                                return hestonP0(u, x, T, params).real();
                            });

    const double P1 = 0.5 + (1.0 / M_PI) * integrate([&](const double u) {
                                return hestonP1(u, x, T, params).real();
                            });

    return dF * (F * P1 - K * P0);
}

/**
 * Heston digital call formula (based on Gatheral's formulation)
 *
 * @param F = forward price
 * @param K = strike price
 * @param T = time to maturity
 * @param dF = discount factor
 * @param params = Heston parameters
 * @return digital call price
 */
inline double hestonDigitalCallFormula(const double F, const double K, const double T,
                                       const double dF, const HestonParams& params) {
    using namespace numerics::integration;

    const double x = std::log(F / K);

    const double P0 = 0.5 + (1.0 / M_PI) * integrate([&](const double u) {
                                return hestonP0(u, x, T, params).real();
                            });

    return dF * P0;
}
}  // namespace pricer