#pragma once

#include <cassert>
#include <random>

#include "mc/Process.h"

// TODO move this to numerics, define RNG type enum for RNGFactory
namespace mc {

class RNG {
   public:
    virtual ~RNG() = default;
    explicit RNG(const std::size_t seed = std::random_device{}())
        : _engine(seed), _dist(0.0, 1.0) {}

    virtual void fill(Sample& sample) {
        for (double& value : sample) {
            value = _dist(_engine);
        }
    }

   private:
    std::mt19937 _engine;
    std::normal_distribution<> _dist;
};

class ConstantRNG final : public RNG {
public:
    explicit ConstantRNG(const double val) : RNG(0), _val(val) {}
    ~ConstantRNG() override = default;
    void fill(Sample& sample) override {
        for (double& value : sample) {
            value = _val;
        }
    }

private:
    double _val;
};

// TODO wrap baseRNG
class AntitheticRNG final : public RNG {
public:
    explicit AntitheticRNG(const std::size_t seed = std::random_device{}()) : RNG(seed) {}

    void fill(Sample& sample) override {
        assert(sample.size() % 2 == 0);
        const std::size_t half = sample.size() / 2;

        Sample half_sample(0.0, half);
        RNG::fill(half_sample);

        sample[std::slice(0, half, 1)] = half_sample;
        sample[std::slice(half, half, 1)] = -half_sample;
    }
};

// TODO define QuasiRNG (Sobol)
}  // namespace mc