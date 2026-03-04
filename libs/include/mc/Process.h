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
    virtual const Sample spot(const StateType& state) const = 0;
};

struct GBMState {
    Sample logF;
};

class GBMProcess final : public Process<GBMState> {
   public:
    GBMProcess(const double F0, const double vol) : _logF0(std::log(F0)), _vol(vol) {}

    GBMState initialState(const std::size_t nPaths) const override {
        return {Sample(_logF0, nPaths)};
    }

    std::size_t nNormals() const override { return 1; }

    GBMState step(const GBMState& currentState, const double, const double dt,
                  const std::vector<Sample>& dW) const override {
        const double drift = -0.5 * _vol * _vol * dt;
        const double diffusion = _vol * std::sqrt(dt);

        return {currentState.logF + drift + diffusion * dW[0]};
    }

    const Sample spot(const GBMState& state) const override { return exp(state.logF); }

   private:
    const double _logF0, _vol;
};

// TODO define LocalVolProcess
struct HestonState {
    Sample logS;
    Sample v;
};

// TODO simulate forward price
class HestonProcess final : public Process<HestonState> {
   public:
    HestonProcess(const double S0, const double v0, const double r, const double kappa,
                  const double theta, const double xi, const double rho)
        : _logS0(std::log(S0)),
          _v0(v0),
          _r(r),
          _kappa(kappa),
          _theta(theta),
          _xi(xi),
          _rho(rho),
          _rhoBar(std::sqrt(1.0 - rho * rho)) {}

    std::size_t nNormals() const override { return 2; }

    HestonState initialState(const std::size_t nPaths) const override {
        return {Sample(_logS0, nPaths), Sample(_v0, nPaths)};
    }

    // dW[0], dW[1] are independent N(0,1)
    // dW_S = dW[0]
    // dW_v = rho * dW[0] + rhoBar * dW[1]   (Cholesky)
    //
    // log(S_{t+dt}) = log(S_t) + (r - 0.5 * v_t) * dt + sqrt(v_t * dt) * dW_S
    // v_{t+dt}      = v_t + kappa*(theta - v_t)*dt + xi*sqrt(v_t * dt) * dW_v
    HestonState step(const HestonState& currentState, const double, const double dt,
                     const std::vector<Sample>& dW) const override {
        const double sqrtDt = std::sqrt(dt);

        // max(v, 0) = (v + |v|) / 2
        const Sample vPos = (currentState.v + abs(currentState.v)) * 0.5;
        const Sample sqrtV = sqrt(vPos);

        // correlated Brownians via Cholesky
        const Sample dW_S = dW[0];
        const Sample dW_v = _rho * dW[0] + _rhoBar * dW[1];

        const Sample logS_next = currentState.logS + (_r - 0.5 * vPos) * dt + sqrtV * sqrtDt * dW_S;
        const Sample v_next = vPos + _kappa * (_theta - vPos) * dt + _xi * sqrtV * sqrtDt * dW_v;

        // floor v_next too
        return {logS_next, (v_next + abs(v_next)) * 0.5};
    }

    const Sample spot(const HestonState& state) const override { return exp(state.logS); }

   private:
    const double _logS0, _v0, _r;
    const double _kappa, _theta, _xi;
    const double _rho, _rhoBar;  // rhoBar precomputed once in constructor
};
}  // namespace mc