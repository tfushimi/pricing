#include "mc/Process.h"

#include <cmath>

namespace mc {

// GBMProcess

GBMProcess::GBMProcess(ForwardCurve forward, const double vol)
    : _forward(std::move(forward)), _vol(vol) {}

GBMState GBMProcess::initialState(const std::size_t nPaths) const {
    return {Sample(0.0, nPaths)};
}

std::size_t GBMProcess::nNormals() const { return 1; }

// log(Z_{t+dt}) - log(Z_t) = vol * (W_{t+dt} - W_t) = vol * N(0, dt)
// S_{t+dt} = F_{t+dt} * exp(Z_{t+dt}) / mean(exp(Z_{t+dt}))
GBMState GBMProcess::step(const GBMState& currentState, const double, const double dt,
                           const std::vector<Sample>& dW) const {
    return {currentState.logZ + (-0.5 * _vol * _vol * dt) + _vol * std::sqrt(dt) * dW[0]};
}

const Sample GBMProcess::value(const GBMState& state, const double time) const {
    return _forward(time) * exp(state.logZ);
}

// LocalVolProcess

LocalVolProcess::LocalVolProcess(ForwardCurve forward, LocalVolFunction localVol)
    : _forward(std::move(forward)), _localVol(std::move(localVol)) {}

LocalVolState LocalVolProcess::initialState(const std::size_t nPaths) const {
    return {Sample(0.0, nPaths)};
}

std::size_t LocalVolProcess::nNormals() const { return 1; }

// log(Z_{t+dt}) - log(Z_t) = localVol(logZ, t) * (W_{t+dt} - W_t) = localVol(logZ, t) * N(0, dt)
// S_{t+dt} = F_{t+dt} * exp(Z_{t+dt}) / mean(exp(Z_{t+dt}))
LocalVolState LocalVolProcess::step(const LocalVolState& currentState, const double currentTime,
                                    const double dt, const std::vector<Sample>& dW) const {
    const auto localVol = _localVol(currentState.logZ, currentTime + dt);
    return {currentState.logZ + (-0.5 * localVol * localVol * dt) +
            localVol * std::sqrt(dt) * dW[0]};
}

const Sample LocalVolProcess::value(const LocalVolState& state, const double time) const {
    return _forward(time) * exp(state.logZ);
}

// HestonProcess

HestonProcess::HestonProcess(ForwardCurve forward, const HestonParams& params)
    : _forward(std::move(forward)),
      _v0(params.v0),
      _kappa(params.kappa),
      _theta(params.theta),
      _xi(params.xi),
      _rho(params.rho),
      _rhoBar(std::sqrt(1.0 - params.rho * params.rho)) {}

std::size_t HestonProcess::nNormals() const { return 2; }

HestonState HestonProcess::initialState(const std::size_t nPaths) const {
    return {Sample(0.0, nPaths), Sample(_v0, nPaths)};
}

// dW[0], dW[1] are independent N(0,1)
// dW_S = dW[0]
// dW_v = rho * dW[0] + rhoBar * dW[1]   (Cholesky)
//
// log(Z_{t+dt}) = log(Z_t) - 0.5 * v_t * dt + sqrt(v_t * dt) * dW_Z
// v_{t+dt}      = v_t + kappa*(theta - v_t)*dt + xi*sqrt(v_t * dt) * dW_v
HestonState HestonProcess::step(const HestonState& currentState, const double, const double dt,
                    const std::vector<Sample>& dW) const {
    const double sqrtDt = std::sqrt(dt);

    // max(v, 0) = (v + |v|) / 2
    const Sample vPos = (currentState.v + abs(currentState.v)) * 0.5;
    const Sample sqrtV = sqrt(vPos);

    // correlated Brownians via Cholesky
    const Sample dW_Z = dW[0];
    const Sample dW_v = _rho * dW[0] + _rhoBar * dW[1];

    const Sample logZ_next = currentState.logZ - vPos * 0.5 * dt + sqrtV * sqrtDt * dW_Z;

    // Milstein schema
    const Sample v_next = vPos + _kappa * (_theta - vPos) * dt + _xi * sqrtV * sqrtDt * dW_v +
                          0.25 * _xi * _xi * (dW_v * dW_v * dt - dt);

    // floor v_next too
    return {logZ_next, (v_next + abs(v_next)) * 0.5};
}

const Sample HestonProcess::value(const HestonState& state, const double time) const {
    return _forward(time) * exp(state.logZ);
}

}  // namespace mc
