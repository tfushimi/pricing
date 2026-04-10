#pragma once

#include <cassert>
#include <memory>
#include <random>

#include "common/Types.h"

namespace rng {

class RNG {
   public:
    virtual ~RNG() = default;
    virtual void fill(Sample& sample) = 0;
};

class NormalRNG final : public RNG {
   public:
    explicit NormalRNG(const std::size_t seed = 0)
        : _engine(seed), _dist(0.0, 1.0) {}
    ~NormalRNG() override = default;
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
    explicit ConstantRNG(const double val) : _val(val) {}
    ~ConstantRNG() override = default;
    void fill(Sample& sample) override {
        for (double& value : sample) {
            value = _val;
        }
    }

   private:
    double _val;
};

class AntitheticRNG final : public RNG {
   public:
    explicit AntitheticRNG(std::unique_ptr<RNG> rng) : _rng(std::move(rng)) {}
    ~AntitheticRNG() override = default;
    void fill(Sample& sample) override {
        assert(sample.size() % 2 == 0);
        const std::size_t half = sample.size() / 2;

        Sample half_sample(0.0, half);
        _rng->fill(half_sample);

        sample[std::slice(0, half, 1)] = half_sample;
        sample[std::slice(half, half, 1)] = -half_sample;
    }

   private:
    std::unique_ptr<RNG> _rng;
};

// TODO define QuasiRNG (Sobol)
}  // namespace rng