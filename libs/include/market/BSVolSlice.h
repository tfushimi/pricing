#pragma once

#include <cmath>

#include "SVI.h"
#include "common/Date.h"
#include "common/Types.h"

namespace market {

// Abstract implied volatility slice at a single expiry. Implementations provide the vol smile
// as a function of strike, used by BS pricers and digital call pricing via dVol/dStrike.
class BSVolSlice {
   public:
    BSVolSlice(const double forward, const double time) : _forward(forward), _time(time) {
        if (_forward <= 0.0) {
            throw std::invalid_argument("BSVolSlice: forward must be positive");
        }
        if (_time <= 0.0) {
            throw std::invalid_argument("BSVolSlice: time must be positive");
        }
    }

    virtual ~BSVolSlice() = default;

    // Returns the forward price of the underlying at this expiry.
    double forward() const { return _forward; }

    // Returns the time to expiry in years.
    double time() const { return _time; }

    // Returns the Black-Scholes implied volatility at the given strike.
    virtual double vol(double strike) const = 0;

    // Returns dVol/dStrike. Defaults to central finite difference; override for analytic gradient.
    virtual double dVolDStrike(const double strike) const {
        constexpr double dK = 1e-4;
        return (vol(strike + dK) - vol(strike - dK)) / (2.0 * dK);
    }

   private:
    double _forward;
    double _time;
};

// Represents a point on a volatility surface
struct VolPoint {
    calendar::Date date;
    const double strike{};
    const double vol{};
};

class FlatVolSlice final : public BSVolSlice {
   public:
    FlatVolSlice(const double forward, const double time, const double vol)
        : BSVolSlice(forward, time), _vol(vol) {}

    double vol(const double) const override { return _vol; }
    double dVolDStrike(const double) const override { return 0.0; }

   private:
    double _vol;
};

// BSVolSlice backed by an SVI parameterization with analytic dVol/dStrike.
class SVIVolSlice final : public BSVolSlice {
   public:
    SVIVolSlice(const double forward, const double time, SVIParams params)
        : BSVolSlice(forward, time), _svi(params) {}

    double vol(const double strike) const override {
        if (strike <= 0.0) {
            throw std::invalid_argument("SVIVolSlice: strike must be positive");
        }
        return _svi.impliedVolAtStrike(forward(), strike, time());
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
        const double k = std::log(forward() / strike);
        const double w = _svi.totalVariance(k);
        const double dw = _svi.dTotalVarianceDk(k);
        const double sigma = std::sqrt(w / time());
        return -dw / (2.0 * sigma * time() * strike);
    }

   private:
    vol::SVI _svi;
};
}  // namespace market