#pragma once

#include <functional>
#include <valarray>
#include <vector>

#include "common/Types.h"

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
    virtual const Sample value(const StateType& state, double time) const = 0;
};

struct GBMState {
    Sample logZ;
};

class GBMProcess final : public Process<GBMState> {
   public:
    using ForwardCurve = std::function<double(double)>;
    explicit GBMProcess(ForwardCurve forward, double vol);

    GBMState initialState(std::size_t nPaths) const override;
    std::size_t nNormals() const override;
    GBMState step(const GBMState& currentState, double currentTime, double dt,
                  const std::vector<Sample>& dW) const override;
    const Sample value(const GBMState& state, double time) const override;

   private:
    const ForwardCurve _forward;
    const double _vol;
};

struct LocalVolState {
    Sample logZ;
};

class LocalVolProcess final : public Process<LocalVolState> {
   public:
    using LocalVolFunction = std::function<Sample(const Sample&, double time)>;
    using ForwardCurve = std::function<double(double)>;
    explicit LocalVolProcess(ForwardCurve forward, LocalVolFunction localVol);

    LocalVolState initialState(std::size_t nPaths) const override;
    std::size_t nNormals() const override;
    LocalVolState step(const LocalVolState& currentState, double currentTime, double dt,
                       const std::vector<Sample>& dW) const override;
    const Sample value(const LocalVolState& state, double time) const override;

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
    HestonProcess(ForwardCurve forward, const HestonParams& params);

    std::size_t nNormals() const override;
    HestonState initialState(std::size_t nPaths) const override;
    HestonState step(const HestonState& currentState, double currentTime, double dt,
                     const std::vector<Sample>& dW) const override;
    const Sample value(const HestonState& state, double time) const override;

   private:
    const ForwardCurve _forward;
    const double _v0;
    const double _kappa, _theta, _xi;
    const double _rho, _rhoBar;  // rhoBar precomputed once in constructor
};

}  // namespace mc
