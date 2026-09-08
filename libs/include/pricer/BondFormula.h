/*
 * Zero-coupon bond prices in one-factor affine short-rate models.
 *
 * With mu(r) = b + beta * r and var(r) = a + alpha * r, P = exp(A(tau) - B(tau) * r);
 *
 * B' = 1 + beta * B - 0.5 * alpha * B * B, B(0) = 0
 * A' = 0.5 * a * B * B - b * B, A(0) = 0
 *
 * Vasicek: {a, b, alpha, beta} = {sigma^2, kappa * theta, 0, -kappa}
 * CIR: {a, b, alpha, beta} = {0, kappa * theta, sigma^2, -kappa}
 */
#pragma once
#include <algorithm>
#include <cmath>

namespace pricer::bond {

struct Affine {
    double a;
    double b;
    double alpha;
    double beta;
};

struct AB {
    double A;
    double B;
};

inline Affine vasicekCoefficients(const double kappa, const double theta, const double sigma) {
    return {.a = sigma * sigma, .b = kappa * theta, .alpha = 0.0, .beta = -kappa};
}

inline Affine cirCoefficients(const double kappa, const double theta, const double sigma) {
    return {.a = 0.0, .b = kappa * theta, .alpha = sigma * sigma, .beta = -kappa};
}

inline AB vasicekAB(const double kappa, const double theta, const double sigma, const double tau) {
    const double B = (1.0 - std::exp(-kappa * tau)) / kappa;
    const double Rinf = theta - sigma * sigma / (2.0 * kappa * kappa);
    const double A = -Rinf * (tau - B) - sigma * sigma * B * B / (4.0 * kappa);
    return {.A = A, .B = B};
}

inline AB cirAB(const double kappa, const double theta, const double sigma, const double tau) {
    const double g = std::sqrt(kappa * kappa + 2.0 * sigma * sigma);
    const double D = (g + kappa) * (std::exp(g * tau) - 1.0) + 2.0 * g;
    const double B = 2.0 * (std::exp(g * tau) - 1.0) / D;
    const double num = 2.0 * g * std::exp((g + kappa) * tau / 2.0);
    const double A = (2.0 * kappa * theta / (sigma * sigma)) * std::log(num / D);
    return {.A = A, .B = B};
}

inline double affineBondPrice(const AB& ab, const double r0) {
    return std::exp(ab.A - ab.B * r0);
}

inline double vasicekFormula(const double kappa, const double theta, const double sigma, const double tau, const double r0) {
    const AB ab = vasicekAB(kappa, theta, sigma, tau);
    return affineBondPrice(ab, r0);
}

inline double cirFormula(const double kappa, const double theta, const double sigma, const double tau, const double r0) {
    const AB ab = cirAB(kappa, theta, sigma, tau);
    return affineBondPrice(ab, r0);
}
}