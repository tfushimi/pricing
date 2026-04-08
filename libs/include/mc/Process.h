#pragma once

#include <functional>
#include <vector>
#include <valarray>

#include "common/types.h"

namespace mc {

// N path values at one fixing date
using Sample = std::valarray<double>;

template <typename StateType>
class Process {
   public:
    using State = StateType;
    virtual ~Process() = default;
    virtual StateType initialState(std::size_t nPaths) const = 0;
    virtual std::size_t nNormals() const = 0;
    virtual StateType step(const StateType& currentState, double currentTime, double dt,
                           const std::vector<Sample>& dW) const = 0;
    virtual const Sample value(const StateType& state, double time) const = 0;
};

// log(Z_{t+dt}) - log(Z_t) = vol * (W_{t+dt} - W_t) = vol * N(0, dt)
// S_{t+dt} = F_{t+dt} * exp(Z_{t+dt}) / mean(exp(Z_{t+dt}))
struct GBMState {
    Sample logZ;
};

class GBMProcess final : public Process<GBMState> {
   public:
    using ForwardCurve = std::function<double(double)>;
    explicit GBMProcess(ForwardCurve forward, const double vol)
        : _forward(std::move(forward)), _vol(vol) {}

    GBMState initialState(const std::size_t nPaths) const override { return {Sample(0.0, nPaths)}; }

    std::size_t nNormals() const override { return 1; }

    GBMState step(const GBMState& currentState, const double, const double dt,
                  const std::vector<Sample>& dW) const override {
        return {currentState.logZ + (-0.5 * _vol * _vol * dt) + _vol * std::sqrt(dt) * dW[0]};
    }

    const Sample value(const GBMState& state, const double time) const override {
        return _forward(time) * exp(state.logZ);
    }

   private:
    const ForwardCurve _forward;
    const double _vol;
};

// log(Z_{t+dt}) - log(Z_t) = localVol(logZ, t), t * (W_{t+dt} - W_t) = localVol(logZ, t) * N(0, dt)
// S_{t+dt} = F_{t+dt} * exp(Z_{t+dt}) / mean(exp(Z_{t+dt}))
struct LocalVolState {
    Sample logZ;
};

class LocalVolProcess final : public Process<LocalVolState> {
public:
    using LocalVolFunction = std::function<Sample(const Sample&, double time)>;
    using ForwardCurve = std::function<double(double)>;
    explicit LocalVolProcess(ForwardCurve forward,
        const LocalVolFunction& localVol)
        : _forward(std::move(forward)), _localVol(localVol) {}

    LocalVolState initialState(const std::size_t nPaths) const override { return {Sample(0.0, nPaths)}; }

    std::size_t nNormals() const override { return 1; }

    LocalVolState step(const LocalVolState& currentState, const double currentTime, const double dt,
                  const std::vector<Sample>& dW) const override {
        const auto localVol = _localVol(currentState.logZ, currentTime + dt);
        return {currentState.logZ + (-0.5 * localVol * localVol * dt) + localVol * std::sqrt(dt) * dW[0]};
    }

    const Sample value(const LocalVolState& state, const double time) const override {
        return _forward(time) * exp(state.logZ);
    }

private:
    const ForwardCurve _forward;
    const LocalVolFunction _localVol;
};

struct HestonState {
    Sample logZ;
    Sample v;
};

class HestonProcess final : public Process<HestonState> {
   public:
    using ForwardCurve = std::function<double(double)>;
    HestonProcess(ForwardCurve forward, const HestonParams& params)
        : _forward(std::move(forward)),
          _v0(params.v0),
          _kappa(params.kappa),
          _theta(params.theta),
          _xi(params.xi),
          _rho(params.rho),
          _rhoBar(std::sqrt(1.0 - params.rho * params.rho)) {}

    std::size_t nNormals() const override { return 2; }

    HestonState initialState(const std::size_t nPaths) const override {
        return {Sample(0.0, nPaths), Sample(_v0, nPaths)};
    }

    // dW[0], dW[1] are independent N(0,1)
    // dW_S = dW[0]
    // dW_v = rho * dW[0] + rhoBar * dW[1]   (Cholesky)
    //
    // log(Z_{t+dt}) = log(Z_t) - 0.5 * v_t * dt + sqrt(v_t * dt) * dW_Z
    // v_{t+dt}      = v_t + kappa*(theta - v_t)*dt + xi*sqrt(v_t * dt) * dW_v
    HestonState step(const HestonState& currentState, const double, const double dt,
                     const std::vector<Sample>& dW) const override {
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

    const Sample value(const HestonState& state, const double time) const override {
        return _forward(time) * exp(state.logZ);
    }

   private:
    const ForwardCurve _forward;
    const double _v0;
    const double _kappa, _theta, _xi;
    const double _rho, _rhoBar;  // rhoBar precomputed once in constructor
};
}  // namespace mc