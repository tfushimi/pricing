#pragma once

#include <map>
#include <valarray>

#include "Date.h"

// log(Z_{t+dt}) = log(Z_t) - 0.5 * v_t * dt + sqrt(v_t * dt) * dW_Z
// v_{t+dt}      = v_t + kappa * (theta - v_t) * dt + xi * sqrt(v_t * dt) * dW_v
struct HestonParams {
    double v0;     // initial value of volatility
    double kappa;  // mean reversion
    double theta;  // long term mean of volatility
    double xi;     // vol of vol
    double rho;    // correlation
};

struct SVIParams {
    double a;      // vertical shift   — overall variance level
    double b;      // slope/width      — controls wing steepness
    double rho;    // skew             — correlation (-1 < rho < 1)
    double m;      // horizontal shift — location of minimum
    double sigma;  // curvature        — smoothness of minimum (> 0)

    void validate() const {
        if (b < 0.0) {
            throw std::invalid_argument("SVI: b must be >= 0");
        }
        if (std::abs(rho) >= 1.0) {
            throw std::invalid_argument("SVI: |rho| must be < 1");
        }
        if (sigma <= 0.0) {
            throw std::invalid_argument("SVI: sigma must be > 0");
        }
        if (a + b * sigma * std::sqrt(1.0 - rho * rho) < 0.0) {
            throw std::invalid_argument("SVI: total variance must be non-negative");
        }
    }
};

// N path values at one fixing date
using Sample = std::valarray<double>;

// full evolution across fixing dates
using Scenario = std::map<calendar::Date, Sample>;
