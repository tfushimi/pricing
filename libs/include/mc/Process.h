#pragma once

#include <iostream>

#include "common/types.h"
namespace mc {

template <typename StateType>
class Process {
   public:
    using State = StateType;
    virtual ~Process() = default;
    virtual StateType initialState(std::size_t nPaths) const = 0;
    virtual std::size_t nNormals() const = 0;
    virtual StateType step(const StateType& currentState, double currentTime, double dt,
                           const std::vector<Sample>& dW) const = 0;
    virtual const Sample value(const StateType& state) const = 0;
};

// log(Z_{t+dt}) - log(Z_t) = vol * (W_{t+dt} - W_t) = vol * N(0, dt)
struct GBMState {
    Sample logZ;
};

class GBMProcess final : public Process<GBMState> {
   public:
    explicit GBMProcess(const double vol) : _vol(vol) {}

    GBMState initialState(const std::size_t nPaths) const override { return {Sample(0.0, nPaths)}; }

    std::size_t nNormals() const override { return 1; }

    GBMState step(const GBMState& currentState, const double, const double dt,
                  const std::vector<Sample>& dW) const override {
        return {currentState.logZ + _vol * std::sqrt(dt) * dW[0]};
    }

    const Sample value(const GBMState& state) const override {
        const auto Z = exp(state.logZ);
        const auto avg = Z.sum() / Z.size();

        return Z / avg;
    }

   private:
    const double _vol;
};

// TODO define LocalVolProcess
struct HestonState {
    Sample logZ;
    Sample v;
};

class HestonProcess final : public Process<HestonState> {
   public:
    HestonProcess(const double v0, const double kappa, const double theta, const double xi,
                  const double rho)
        : _v0(v0),
          _kappa(kappa),
          _theta(theta),
          _xi(xi),
          _rho(rho),
          _rhoBar(std::sqrt(1.0 - rho * rho)) {}

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

        const Sample logZ_next = currentState.logZ + sqrtV * sqrtDt * dW_Z;
        const Sample v_next = vPos + _kappa * (_theta - vPos) * dt + _xi * sqrtV * sqrtDt * dW_v;

        // floor v_next too
        return {logZ_next, (v_next + abs(v_next)) * 0.5};
    }

    const Sample value(const HestonState& state) const override {
        const auto Z = exp(state.logZ);
        const auto avg = Z.sum() / Z.size();

        return Z / avg;
    }

   private:
    const double _v0;
    const double _kappa, _theta, _xi;
    const double _rho, _rhoBar;  // rhoBar precomputed once in constructor
};
}  // namespace mc