#pragma once

#include <valarray>
#include <map>

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

// N path values at one fixing date
using Sample = std::valarray<double>;

// full evolution across fixing dates
using Scenario = std::map<calendar::Date, Sample>;
