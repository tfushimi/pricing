#pragma once

#include <cmath>
#include <complex>
#include <functional>

namespace pricer {
namespace {

// TODO can we consolidate P0 and P1 somehow?
std::complex<double> hestonP0(const double x, const double u, const double v0,
                                     const double T, const double kappa, const double theta,
                                     const double xi, const double rho) {
    using Complex = std::complex<double>;
    const Complex iu{0.0, u};

    const Complex alpha = -0.5 * u * u - 0.5 * iu;
    const Complex beta = kappa - rho * xi * iu;
    const Complex d = sqrt(beta * beta - 2.0 * alpha * xi * xi);
    const Complex rMinus = (beta - d) / (xi * xi);
    const Complex rPlus = (beta + d) / (xi * xi);
    const Complex g = rMinus / rPlus;
    const Complex expdt = exp(-d * T);

    const Complex C = kappa * (rMinus * T - (2.0 / (xi * xi)) * log((1.0 - g * expdt) / (1.0 - g)));
    const Complex D = rMinus * (1.0 - expdt) / (1.0 - g * expdt);

    return exp(C * theta + D * v0 + iu * x) / iu;
}

std::complex<double> hestonP1(const double x, const double u, const double v0,
                                     const double T, const double kappa, const double theta,
                                     const double xi, const double rho) {
    using Complex = std::complex<double>;
    const Complex iu{0.0, u};

    const Complex alpha = -0.5 * u * u + 0.5 * iu;
    const Complex beta = kappa - rho * xi * iu - rho * xi;
    const Complex d = sqrt(beta * beta - 2.0 * alpha * xi * xi);
    const Complex rMinus = (beta - d) / (xi * xi);
    const Complex rPlus = (beta + d) / (xi * xi);
    const Complex g = rMinus / rPlus;
    const Complex expdt = exp(-d * T);

    const Complex C = kappa * (rMinus * T - (2.0 / (xi * xi)) * log((1.0 - g * expdt) / (1.0 - g)));
    const Complex D = rMinus * (1.0 - expdt) / (1.0 - g * expdt);

    return exp(C * theta + D * v0 + iu * x) / iu;
}
}

// TODO move this to numerics/integration
inline double integrate(const std::function<double(double)>& f) {
    constexpr double du = 0.001;
    constexpr double uMax = 1000.0;
    double sum = 0.0;
    for (double u = du; u < uMax; u += du) sum += f(u) * du;
    return sum;
}

inline double hestonCallFormula(const double F, const double K, const double T, const double dF,
                                const double v0, const double kappa, const double theta,
                                const double xi, const double rho) {
    const double x = std::log(F / K);  // log-moneyness, rate already absorbed into F

    const double P0 = 0.5 + (1.0 / M_PI) * integrate([&](const double u) {
                                return hestonP0(x, u, v0, T, kappa, theta, xi, rho).real();
                            });

    const double P1 = 0.5 + (1.0 / M_PI) * integrate([&](const double u) {
                                return hestonP1(x, u, v0, T, kappa, theta, xi, rho).real();
                            });

    return dF * (F * P1 - K * P0);
}
}  // namespace pricer