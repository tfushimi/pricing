#pragma once

#include <cmath>

namespace bs {
inline double normCdf(const double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

inline double normPdf(const double x) {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * M_PI);
}

inline std::pair<double, double> blackD1D2(double F, double K, double T, double sigma) {
    const double d1 = (std::log(F / K) + 0.5 * sigma * sigma * T) / (sigma * std::sqrt(T));
    return {d1, d1 - sigma * std::sqrt(T)};
}

inline double blackCallFormula(const double F, const double K, const double T, const double dF,
                               const double vol) {
    const auto [d1, d2] = blackD1D2(F, K, T, vol);
    return dF * (F * normCdf(d1) - K * normCdf(d2));
}

inline double blackVega(const double F, const double K, const double T, const double dF,
                        const double vol) {
    const auto [d1, d2] = blackD1D2(F, K, T, vol);

    return dF * F * normPdf(d1) * std::sqrt(T);
}

inline double blackDigitalFormula(const double F, const double K, const double T, const double dF,
                                  const double vol, const double dVolDStrike) {
    const auto [d1, d2] = blackD1D2(F, K, T, vol);

    return dF * normCdf(d2) - blackVega(F, K, T, dF, vol) * dVolDStrike;
}
}  // namespace bs