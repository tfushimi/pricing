#pragma once

#include <cmath>
#include <stdexcept>
#include <string>

#include "common/Types.h"

namespace vol {

/**
 * Computes implied vol from SVIParams at a given maturity
 */
class SVI {
   public:
    explicit SVI(SVIParams params) : _params(params) { _params.validate(); }

    // Total implied variance: w(k) = sigma_imp^2 * T
    // k = log(F/K) = log-moneyness
    double totalVariance(const double k) const {
        const double dk = k - _params.m;
        const double disc = std::sqrt(dk * dk + _params.sigma * _params.sigma);
        return _params.a + _params.b * (_params.rho * dk + disc);
    }

    // First derivative dw/dk — needed for skew and local vol
    double dTotalVarianceDk(const double k) const {
        const double dk = k - _params.m;
        const double disc = std::sqrt(dk * dk + _params.sigma * _params.sigma);
        return _params.b * (_params.rho + dk / disc);
    }

    // Second derivative d^2w/dk^2 — needed for density
    double d2TotalVarianceDk2(const double k) const {
        const double dk = k - _params.m;
        const double disc = std::sqrt(dk * dk + _params.sigma * _params.sigma);
        return _params.b * _params.sigma * _params.sigma / (disc * disc * disc);
    }

    // Implied vol at log-moneyness k and maturity T
    double impliedVol(const double k, const double T) const {
        const double w = totalVariance(k);
        if (w < 0.0) {
            throw std::runtime_error("SVI: negative total variance at k=" + std::to_string(k));
        }

        return std::sqrt(w / T);
    }

    // Implied vol at strike K given forward F and maturity T
    double impliedVolAtStrike(const double F, const double K, const double T) const {
        if (K <= 0.0) {
            throw std::invalid_argument("SVI: strike must be positive");
        }
        return impliedVol(std::log(F / K), T);
    }

   private:
    SVIParams _params;
};

}  // namespace vol