#pragma once

#include <cmath>
#include <stdexcept>

#include "market/BSVolSlice.h"
#include "vol/SVI.h"

namespace market {

/**
 * Implements BSVolSlice using SVI parameterization
 */
class SVIVolSlice final : public BSVolSlice {
   public:
    SVIVolSlice(const double forward, const double time, vol::SVIParams params)
        : _forward(forward), _time(time), _svi(std::move(params)) {
        if (_forward <= 0.0) {
            throw std::invalid_argument("SVIVolSlice: forward must be positive");
        }
        if (_time <= 0.0) {
            throw std::invalid_argument("SVIVolSlice: time must be positive");
        }
    }

    double forward() const override { return _forward; }
    double time() const override { return _time; }

    double vol(const double strike) const override {
        if (strike <= 0.0) {
            throw std::invalid_argument("SVIVolSlice: strike must be positive");
        }
        return _svi.impliedVolAtStrike(_forward, strike, _time);
    }

    /**
     * Analytical dSigma/dK via chain rule:
     *   k      = log(F/K)       →  dk/dK = -1/K
     *   sigma  = sqrt(w(k)/T)   →  dsigma/dk = dw/dk / (2*sigma*T)
     *   dsigma/dK = dsigma/dk * dk/dK = -dw/dk / (2*sigma*T*K)
     */
    double dVolDStrike(double strike) const override {
        if (strike <= 0.0) {
            throw std::invalid_argument("SVIVolSlice: strike must be positive");
        }
        const double k = std::log(_forward / strike);
        const double w = _svi.totalVariance(k);
        const double dw = _svi.dTotalVarianceDk(k);
        const double sigma = std::sqrt(w / _time);
        return -dw / (2.0 * sigma * _time * strike);
    }

   private:
    double _forward;
    double _time;
    vol::SVI _svi;
};

}  // namespace market